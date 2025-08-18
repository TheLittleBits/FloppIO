import sys, math

def show_progress(time, length):
    percentage = '\rProgress: ' + str(math.floor((time / length) * 100)) + '%'
    sys.stdout.write(percentage)
    sys.stdout.flush()