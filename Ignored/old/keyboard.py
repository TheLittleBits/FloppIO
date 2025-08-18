#  ______ _                  _____ ____  
# |  ____| |                |_   _/ __ \ 
# | |__  | | ___  _ __  _ __  | || |  | |
# |  __| | |/ _ \| '_ \| '_ \ | || |  | |
# | |    | | (_) | |_) | |_) || || |__| |
# |_|    |_|\___/| .__/| .__/_____\____/ 
#                | |   | |               
#                |_|   |_|               
#
#           Made by Cosmin Piriu
#
#
# FDD, HDD and scanner music using MIDI Keyboards

print('Loading modules...', end='')
import mido
import driver
print('DONE')

print('''
  ______ _                  _____ ____  
 |  ____| |                |_   _/ __ \\ 
 | |__  | | ___  _ __  _ __  | || |  | |
 |  __| | |/ _ \\| '_ \\| '_ \\ | || |  | |
 | |    | | (_) | |_) | |_) || || |__| |
 |_|    |_|\\___/| .__/| .__/_____\\____/ 
                | |   | |               
                |_|   |_|               
      ''')

# Open the keyboard device
try:
    device = mido.get_input_names()[1]
except IndexError:
    print('No device found. Closing up.')
    driver.cleanup
    exit()

print('Initialising...', end = '')
driver.init() # Init the driver
print('DONE')

def handle_msg(msg):
    if msg.type == 'pitchwheel':
        driver.pitchwheel(msg.channel, msg.pitch)
    if msg.type == 'note_on':
        driver.fdd(msg.channel, msg.note, msg.velocity) # Play a note with the fdd
    if msg.type == 'note_off':
        driver.fdd(msg.channel, msg.note, 0) # Stop playing a note with the fdd

def main():
    global device
    with mido.open_input(device) as input:
        for msg in input:
            handle_msg(msg)
            print(msg)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt: # In case of keyboard interruption
        print("\nInterrupted by user. Closing up.")
        driver.cleanup() # Disables the gpio pins
        exit()
    except SystemExit:
        driver.cleanup() # Disables the gpio pins
        exit()
    except: # In case of other errors
        print("\nAn error occured. Closing up.")
        driver.cleanup() # Disables the gpio pins
        exit()