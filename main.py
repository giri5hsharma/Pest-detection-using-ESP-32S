import tensorflow as tf
import tensorflow.keras.layers as layers
import tensorflow.keras.models as models

# Loading pest dataset
dataset_dir = "/Users/girishsharma/Desktop/tinker2/pest"  # Update with your dataset path
image_size = (32, 32)  # Small size for ESP32-CAM
batch_size = 32

#preprocessing  images
train_ds = tf.keras.preprocessing.image_dataset_from_directory(
    dataset_dir,
    validation_split=0.2,
    subset="training",
    seed=123,
    image_size=image_size,
    batch_size=batch_size
)
val_ds = tf.keras.preprocessing.image_dataset_from_directory(
    dataset_dir,
    validation_split=0.2,
    subset="validation",
    seed=123,
    image_size=image_size,
    batch_size=batch_size
)

# Normalizing  images ||| 255--> 0
train_ds = train_ds.map(lambda x, y: (x / 255.0, y))
val_ds = val_ds.map(lambda x, y: (x / 255.0, y))

# Build small CNN
model = models.Sequential([
    layers.Conv2D(8, (3, 3), activation='relu', input_shape=(32, 32, 3)),
    layers.MaxPooling2D((2, 2)),
    layers.Conv2D(16, (3, 3), activation='relu'),
    layers.MaxPooling2D((2, 2)),
    layers.Flatten(),
    layers.Dense(32, activation='relu'),
    layers.Dense(4, activation='softmax')  # 4 classes: ants, bees, beetles, caterpillars
])

# Compile and train
model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])
model.fit(train_ds, validation_data=val_ds, epochs=10)

# Define representative dataset function for quantization
def representative_data_gen():
    for image, _ in train_ds.take(100):  # Use 100 samples from the training data
        yield [image]

# Convert to TFLite with quantization
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_types = [tf.int8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8
converter.representative_dataset = representative_data_gen

# Converting and save the TFLite model
tflite_model = converter.convert()

# Saving the TFLite model
with open("pest_model.tflite", "wb") as f:
    f.write(tflite_model)



