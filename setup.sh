sudo raspi-config
sudo apt update
sudo apt upgrade -y
sudo apt install python3 python3-serial python3-mido git -y
sudo systemctl disable hciuart
mkdir floppio
cd floppio
git clone https://github.com/TheLittleBits/FloppIO.git
mv FloppIO/*.py ./
mkdir midi
mv FloppIO/example-midi/*.mid midi/
mv FloppIO/pico/floppy/floppy.elf ./
mv FloppIO/pico/scanner/scanner.elf ./
rm FloppIO/ -rf
sudo nano /boot/firmware/config.txt
sudo reboot
