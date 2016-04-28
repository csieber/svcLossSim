# Vagrant Example

## Requirements

 - Vagrant
 - VirtualBox
 
## Setup

Clone the repository, spin up the VM and ssh into the machine:

```bash
git clone https://github.com/csieber/svcLossSim.git
cd svcLossSim/vagrant/

vagrant up
vagrant ssh
```

Inside the machine, download an example video:

```bash
sudo apt-get install unzip

cd /svcLossSim/examples/unzip video_5.zip

wget http://sourceforge.net/projects/opensvcdecoder/files/Video%20Streams/video_5.zip/download -O video_5.zip

unzip video_5.zip
```

Use svclosssim to emulate packet loss on the file:

```bash
svclosssim -l HelloWorld.lua -s video_5/video_5.264 > out.264
```

**Important**: Always use shell output redirection (*>*)! svclosssim returns the emulated output stream directly to the shell.

The output should look like this:

```
LUA: Finished(). drop() was called 2565 times.
Simulation finished.

Total elapsed time: 492 ms.
Total frames processed: 501
Total NAL units processed: 2005
Total Bytes processed: 944091 (including NAL separator)
Total Bytes written: 944091 ( including 0 'zeroed' bytes)

Actions taken (for the 2565 splitted NAL chunks ('packets')):
ACTION_OUTPUT: 2565 (100%)
ACTION_DROP: 0 (0%)
ACTION_ZERO: 0 (0%)
ACTION_DROP_BUT_KEEP_HDR: 0 (0%)
ACTION_ZERO_BUT_KEEP_HDR: 0 (0%)

Settings: 
Maximum chunk ('packet') size: 1000 bytes (excluding NAL separator)
Assumed NAL header size: 20 bytes
```

The hello world example does not do any packet loss, but merely copies the file.

Use *example1_BasicPacketLoss.lua* to emulate a packet loss of 1%.
