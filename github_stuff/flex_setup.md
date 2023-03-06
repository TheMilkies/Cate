# Setting up Modern GNU Flex on Debian based distros.
## Installing dependencies
Run the following in a terminal emulator.
`sudo apt install -y autoconf automake make help2man m4 gcc g++ gettext bison texinfo libtool`

## Cloning and building Flex 2.6.4+
Clone the [Flex repository](https://github.com/westes/flex)

`git clone https://github.com/westes/flex.git`

CD inside the directory

`cd flex/`

Run these commands one after the other and check errors for missing packages:
```sh
./autogen.sh
./configure
make -j`nproc`
sudo make install
```