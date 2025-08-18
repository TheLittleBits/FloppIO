def _get_drum_track(midifile):
    
    ''' Algorithm to get the drum track from a midifile '''

    tracks = midifile.tracks
    track_index = None

    for index, track in enumerate(tracks):
        for msg in track:
            if 'channel' in vars(msg):
                if msg.channel == 9:
                    track_index = index
                    break
        if track_index != None:
            break
    
    return tracks[track_index]

def arrange_drums(midifile):

    ''' This function sorts the drum message notes in 0, 1, 2, 3, etc so the
    HDDs always have to play something. '''

    drum_track = _get_drum_track(midifile)
    all_drums = []

    for msg in drum_track:
        if 'note' in vars(msg):
            if msg.note not in all_drums:
                all_drums.append(msg.note)

    all_drums.sort()
    
    for msg in drum_track:
        if 'note' in vars(msg):
            msg.note = all_drums.index(msg.note) # Rearrange according to the channel's index in the array

    return midifile

def arrange_channels(midifile):

    ''' This function arranges the 16 midi channels so there 
    remain no gaps between channels.
    Example: We have channel 1, 3, 4 and 7. This function goes through
    all of them and rearranges them in 0, 1, 2 and 3.
    This way you can save up editing time and floppies. '''

    tracks = midifile.tracks
    drum_track = _get_drum_track(midifile)
    tracks.remove(drum_track)

    channels = [] # We will put all the midi channels in this array
    for track in tracks:
        for msg in track:
            if 'channel' in vars(msg): # Continue if the midi message has a channel parameter
                if msg.channel not in channels: # Avoid duplications
                    channels.append(msg.channel)

    # Ressamble the channels around 9, so channel 9 stays clear for the drums
    for track in tracks:
        for msg in track:
            if 'channel' in vars(msg):
                if msg.channel < 9:
                    msg.channel = channels.index(msg.channel)
                else:
                    msg.channel = channels.index(msg.channel) + 1

    tracks.append(drum_track) # At last, add the drum track with channel 9

    return midifile # Return the new file