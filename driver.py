import serial as _serial
from math import floor as _floor
from math import log as _log
from time import sleep as _sleep

def _note_to_hz(note):
    # Translate a midi note to hertz
    return 440 * 2 ** ((note - 69) / 12)

def _hz_to_note(hz):
    # Translate hertz to midi notes
    if hz > 0:
        return 12 * (_log(hz/220)/_log(2)) + 57
    elif hz == 0:
        return 0
    else:
        hz *= -1
        return -1 * (12 * (_log(hz/220)/_log(2)) + 57)

def _pitch_to_note(pitch):
    # Translate the midi pitchbend value to midi note value
    return pitch / 4096

def _send(action, value):
    # Send one message over UART
    global port
    value = _floor(value)
    port.write(bytes([action + 128])) # Send the action
    port.write(bytes([_floor(value/127)])) # Send value1
    port.write(bytes([value%127])) # Send value2


def init():
    global port, db
    db = []
    # Open the serial UART port to the pico
    port = _serial.Serial('/dev/serial0', 1000000)
    # Resolve a UART bug on the pico side. Action 2 does
    # nothing, but after a reboot of pico the code skips
    # the first uart message.
    _send(2, 0)
    # Create a database with info about the 16 midi channels
    for i in range(16):
        db.append({"frequency": 0, "frequency_raw": 0, "pitchwheel": 0, "playing": False})

def startup():
    _send(1, 1) # Enable the FDDs and the HDDs

def play(channel, note, velocity):
    if velocity > 0:
        db[channel]["frequency"] = _note_to_hz(note + _pitch_to_note(db[channel]["pitchwheel"])) # Total frequency (midi note frequency + pitchbend)
        db[channel]["frequency_raw"] = _note_to_hz(note) # Only the midi note frequency
        db[channel]["playing"] = True # Whether it it playing or not
        _send(channel + 10, db[channel]["frequency"]) # Send the the channel + 10 (because channel 0 is action 10 in the pico code)
    else:
        if _note_to_hz(note) == db[channel]["frequency_raw"]: # Check if this is the note that should be turned off
            db[channel]["frequency"] = 0 # Frequency is 0, so note is not played
            db[channel]["frequency_raw"] = 0 # Same here
            db[channel]["playing"] = False # Not playing
            _send(channel + 10, 0) # Send the floppy ID plus the "off" value

def hdd(note, velocity):
    if velocity > 0:
        _send(126, note) # Action 126 is a click with the HDD

def pitchwheel(channel, pitch):
    db[channel]["pitchwheel"] = pitch # Set the pitchwheel value
    db[channel]["frequency"] = _note_to_hz(_pitch_to_note(db[channel]["pitchwheel"]) + _hz_to_note(db[channel]["frequency_raw"])) # Recalculate the frequency
    if db[channel]["playing"]: # Without this statement the floppy would play at every pitchwheel message
        _send(channel + 10, db[channel]["frequency"]) # Send the new frequency

def reset():
    _send(0, 0) # Send the reset message
    _sleep(3) # Wait for the floppies to reset

def cleanup():
    # Cleaning up
    global port
    for i in range(10, 20): # Send frequency 0 to all devices to stop playing
        _send(i, 0)
        _sleep(0.01)
    _send(1, 0) # Disable the HDDs
    port.close()
