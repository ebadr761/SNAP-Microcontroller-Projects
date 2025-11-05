import serial
import time
import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime

# ===== CONFIGURATION =====
SERIAL_PORT = '/dev/cu.usbserial-0001'  # Change if needed
BAUD_RATE = 4800
COLLECTION_TIME = 10  # seconds
VREF = 3.3  # Reference voltage
ADC_MAX = 1023  # 10-bit ADC maximum value

def parse_hex_value(line):
    """Parse hex value from format '0x01F8' or ' 0x01F8'"""
    try:
        # Remove whitespace and parse hex
        hex_str = line.strip()
        if '0x' in hex_str:
            return int(hex_str, 16)
        return None
    except:
        return None

def collect_adc_data(port, baud, duration):
    """Collect ADC data from serial port for specified duration"""
    
    print(f"Opening serial port {port} at {baud} baud...")
    
    try:
        ser = serial.Serial(port, baud, timeout=1)
        time.sleep(2)  # Wait for serial to stabilize
        
        print(f"\n{'='*50}")
        print(f"Collecting data for {duration} seconds...")
        print(f"Turn the potentiometer during collection!")
        print(f"{'='*50}\n")
        
        # Clear any initial data
        ser.reset_input_buffer()
        
        # Data storage
        timestamps = []
        adc_values = []
        voltages = []
        
        start_time = time.time()
        
        while (time.time() - start_time) < duration:
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('ascii', errors='ignore')
                    adc_value = parse_hex_value(line)
                    
                    if adc_value is not None:
                        elapsed = time.time() - start_time
                        voltage = (adc_value / ADC_MAX) * VREF
                        
                        timestamps.append(elapsed)
                        adc_values.append(adc_value)
                        voltages.append(voltage)
                        
                        # Print to console
                        print(f"Time: {elapsed:6.2f}s | ADC: {adc_value:4d} (0x{adc_value:03X}) | Voltage: {voltage:.3f}V")
                
                except Exception as e:
                    print(f"Error reading: {e}")
                    continue
        
        ser.close()
        print(f"\n{'='*50}")
        print(f"Data collection complete! Collected {len(adc_values)} samples")
        print(f"{'='*50}\n")
        
        return timestamps, adc_values, voltages
    
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        print("Make sure:")
        print("1. Terminal is closed (not using the port)")
        print("2. PIC is connected")
        print("3. Port name is correct")
        return None, None, None

def create_dataframe(timestamps, adc_values, voltages):
    """Create pandas DataFrame from collected data"""
    
    df = pd.DataFrame({
        'Time (s)': timestamps,
        'ADC Buffer Value': adc_values,
        'Voltage (V)': voltages
    })
    
    # Round for readability
    df['Time (s)'] = df['Time (s)'].round(2)
    df['Voltage (V)'] = df['Voltage (V)'].round(3)
    
    return df

def plot_data(timestamps, adc_values, voltages):
    """Create two separate plots for ADC values and voltages"""
    
    # Create figure with 2 subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    
    # Plot 1: ADC Buffer Value vs Time
    ax1.plot(timestamps, adc_values, 'b-o', linewidth=2, markersize=4)
    ax1.set_xlabel('Time (seconds)', fontsize=12)
    ax1.set_ylabel('ADC Buffer Value', fontsize=12)
    ax1.set_title('ADC Digital Output vs Time', fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.set_ylim([0, 1100])
    
    # Plot 2: Voltage vs Time
    ax2.plot(timestamps, voltages, 'r-o', linewidth=2, markersize=4)
    ax2.set_xlabel('Time (seconds)', fontsize=12)
    ax2.set_ylabel('Voltage (V)', fontsize=12)
    ax2.set_title('ADC Input Voltage vs Time', fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim([0, 3.5])
    
    plt.tight_layout()
    
    # Save figure
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"adc_plots_{timestamp}.png"
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Plots saved as: {filename}")
    
    plt.show()

def main():
    """Main function"""
    
    print("\n" + "="*50)
    print("ADC Data Collection and Plotting")
    print("="*50)
    timestamps, adc_values, voltages = collect_adc_data(SERIAL_PORT, BAUD_RATE, COLLECTION_TIME) #collect data
    
    if timestamps is None or len(timestamps) == 0:
        print("No data collected. Exiting.")
        return
    
    # Create DataFrame
    df = create_dataframe(timestamps, adc_values, voltages)
    
    # Display DataFrame
    print("\nCollected Data:")
    print(df.to_string(index=False))
    
    # Save to CSV
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_filename = f"adc_data_{timestamp}.csv"
    df.to_csv(csv_filename, index=False)
    print(f"\nData saved to: {csv_filename}")
    
    # Plot data
    plot_data(timestamps, adc_values, voltages)

if __name__ == "__main__":
    main()