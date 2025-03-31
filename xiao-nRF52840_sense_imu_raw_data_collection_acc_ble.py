import asyncio
from bleak import BleakClient

# Define the UUID of the characteristic you want to read (same as in Arduino code)
# Replace with your actual UUID for accel data (same as in Arduino code)
ACCEL_DATA_UUID = "2A37"

# Function to handle incoming notifications


def notification_handler(sender, data):
    # Convert the byte data to a string
    accel_data_str = data.decode('utf-8')
    print("Received data:", accel_data_str)


async def run():
    # Define the MAC address or BLE device name (check your device's name or address)
    device_name = "Xiao nRF52840 Sense"  # The name you set in the Arduino code
    device_address = None  # Automatically search for the device

    # Initialize BLE client
    async with BleakClient(device_name or device_address) as client:
        # Check if connected
        connected = await client.is_connected()
        if connected:
            print(f"Connected to {device_name}")
        else:
            print(f"Failed to connect to {device_name}")
            return

        # Subscribe to notifications from the accelerometer data characteristic
        await client.start_notify(ACCEL_DATA_UUID, notification_handler)

        # Keep the program running to listen for notifications
        print("Listening for accelerometer data...")
        # You can change this time based on how long you want to listen
        await asyncio.sleep(60)

        # After listening for a while, stop the notifications
        await client.stop_notify(ACCEL_DATA_UUID)
        print("Stopped listening for data.")

# Run the script
if __name__ == "__main__":
    asyncio.run(run())
