cd /sys/class/gpio
echo 12 > export
cd gpio12
echo out
echo out > direction
cd ..
echo 12 > unexport
cd ~
