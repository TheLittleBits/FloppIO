sudo raspi-config
sudo apt update
sudo apt upgrade -y
sudo apt install python3 python3-serial python3-mido -y
sudo systemctl disable hciuart
mkdir floppio
mv * floppio/
cp floppio/*.py ./
mkdir midi
cp floppio/example-midi/*.mid midi/
cp floppio/pico/floppy/floppy.elf ./
cp floppio/pico/scanner/scanner.elf ./
sudo nano /boot/firmware/config.txt
sudo reboot
