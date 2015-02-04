# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "remram/debian-7-amd64"

  config.vm.provision "shell",
    inline: <<SCRIPT
sudo aptitude update -y
sudo aptitude install -y apache2 libapache2-mod-fcgid git g++
SCRIPT
end
