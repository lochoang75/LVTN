#!/bin/bash
#
# Tool for install all requirement tool for petalinux
# All tool base on petalinux software manual https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_1/ug1144-petalinux-tools-reference-guide.pdf 
#

function log {
    echo $1  
}

function try {
    sudo apt-get -y install $1
    local result=$?
    if [ $result -eq 0 ]
    then
        log "Sucess install $1"
    else
        log "Failed to install $1"
        exit
    fi 
}

# First update current system
sudo apt-get update

# Install dos2unix
try dos2unix

# Install gawk 
try gawk

# Install xvfb
try xvfb

# Install gcc
try gcc

# Install git
try git

# Install make
try make

# Install netstat
try net-tools

# Install ncurses devel
try libncurses5-dev

# Install tftp server
try tftpd

# Install zlib-devel
try zlib1g-dev

# Install zlib1g:i386
try zlib1g:i386

# Install openssl
try libssl-dev

# Install flex
try flex

# Instal bison
try bison

# Install libselinux
try libselinux1

# Insall gnupg
try gnupg

# Install wget
try wget

# Install diffstat
try diffstat

# Install socat
try socat

# Install xterm
try xterm

# Install autoconf
try autoconf

# Install libtool
try libtool

# Install tar
try tar

# Install unzip
try unzip

# Install texinfo
try texinfo

# Install zlib1g-dev
try zlib1g-dev

# Install gcc-multilib
try gcc-multilib

# Install buil-essential
try build-essential

# Install libsdl1.2-dev
try libsdl1.2-dev

# Install libglib2.0-dev
try libglib2.0-dev

# Install screen
try screen

# Install pax
try pax

# Install gzip
try gzip

# Hidden tool chrpath
try chrpath
