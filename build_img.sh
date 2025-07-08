image_name="flash_image.bin"
baud_frequency="115200"

echo "Building image: $image_name with baud frequency: $baud_frequency"

pio run
if [ $? -ne 0 ]; then
    echo "Error during build. Please check the output for details."
    exit 1
fi
echo "Moving files..."
mv .pio/build/az-delivery-devkit-v4/firmware.bin build/firmware.bin
mv .pio/build/az-delivery-devkit-v4/bootloader.bin build/bootloader.bin
mv .pio/build/az-delivery-devkit-v4/partitions.bin build/partitions.bin

# check if esptool.py is installed
if ! command -v esptool.py &> /dev/null; then
    echo "esptool.py could not be found. Please install it using 'pip install esptool'."
    exit 1
fi
echo "Creating image..."
esptool.py --chip esp32 merge_bin -o $image_name --flash_mode dio --flash_freq 40m --flash_size 4MB \
0xe000 app_bin/boot_app0.bin \
0x1000 build/bootloader.bin \
0x8000 build/partitions.bin \
0x10000 build/firmware.bin --fill-flash-size 4MB
if [ $? -ne 0 ]; then
    echo "Error creating image. Please check the output for details."
    exit 1
fi
echo "Image built successfully: $image_name with baud frequency: $baud_frequency"