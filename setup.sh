sudo raspi-config
sudo apt update
sudo apt upgrade -y
sudo apt install python3 python3-serial python3-mido git -y
sudo systemctl disable hciuart
mkdir build
cd build
git clone https://github.com/TheLittleBits/FloppIO.git
mv FloppIO/*.py ./
mkdir midi
mv FloppIO/*.mid midi/
sudo nano /boot/firmware/config.txt
sudo reboot