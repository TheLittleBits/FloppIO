def get_length(file):
    print('Calculating file length...', end='')
    tracks = file.tracks
    length = 0
    for track in tracks:
        for msg in track:
            if msg.time > length:
                length = msg.time
        print('.', end='')
    print('DONE')
    return length