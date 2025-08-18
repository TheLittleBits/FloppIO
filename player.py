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
# FDD, HDD and scanner music using MIDI Files

print('Loading modules...', end='')
import mido
import sys
import driver
from time import sleep
from MidiEdit import arrange_channels, arrange_drums
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

ENABLE_EDIT_CHANNELS = False # Whether arrange the midi channels

# Load the midi file with Mido
try:
    # Use MidiEdit to get ready the midifile for FloppIO
    if ENABLE_EDIT_CHANNELS:
        MidiFile = arrange_channels(arrange_drums(mido.MidiFile(sys.argv[1])))
    else:
        MidiFile = arrange_drums(mido.MidiFile(sys.argv[1]))

except IndexError:
    print('Please specify the midi file.')
    exit()
except OSError:
    print('Invalid file format. Probably not a midi file.')
    exit()

print('Initialising...', end = '')
driver.init() # Init the driver
sleep(1)
print('DONE')

print('Resetting...', end = '')
driver.reset() # Init the driver
sleep(1)
print('DONE')

print('Starting up...', end = '')
driver.startup() # Enable the FDDs and HDDs
sleep(1)
print('DONE')

def handle_msg(msg):
    if msg.type == 'pitchwheel':
        driver.pitchwheel(msg.channel, msg.pitch) # Set pitchbend for a specific channel
    if msg.type == 'note_on':
        if msg.channel == 9:
            driver.hdd(msg.note, msg.velocity) # Click the HDD
        else:
            driver.play(msg.channel, msg.note, msg.velocity) # Play a note with the fdd
    if msg.type == 'note_off':
        driver.play(msg.channel, msg.note, 0) # Stop playing a note with the fdd

def main():
    for msg in MidiFile.play():
        handle_msg(msg) # Sends the message to the message handler
    
    print('\nDone playing file. Goodbye')
    driver.cleanup() # Disables the gpio pins
    exit()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt: # In case of keyboard interruption
        print("\nInterrupted by user. Closing up.")
        driver.cleanup() # Disables the gpio pins
        exit()
    except Exception as error: # In case of other errors
        print("\nAn error occured: {}.\nClosing up.".format(error))
        driver.cleanup() # Disables the gpio pins
        exit()
