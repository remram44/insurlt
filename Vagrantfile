# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "remram/debian-7-amd64"

  config.vm.provision "shell",
    inline: <<SCRIPT
sudo aptitude update -y
sudo aptitude install -y g++ cmake libsqlite3-dev libcppunit-dev libfcgi-dev
sudo -u vagrant sh <<NESTED
cd /home/vagrant
mkdir build
cd build
cmake /vagrant -DBUILD_TESTS=on
make
./tests
NESTED
sudo aptitude install -y apache2 libapache2-mod-fcgid
sudo a2enmod fcgid rewrite
sudo cat > /etc/apache2/sites-available/insurlt <<'NESTED'
<VirtualHost *>
    ServerAdmin webmaster@localhost

    DocumentRoot /home/vagrant/build/public/

    RewriteEngine on
    RewriteCond %{REQUEST_FILENAME} !-d
    RewriteCond %{REQUEST_FILENAME} !-f
    RewriteRule ^(.*)$ /insurlt.fcgi [L,H=fcgid-script]

    FcgidInitialEnv DATABASE_PATH /tmp/database.sqlite3

    <Location />
        AddHandler fcgid-script .fcgi
        Options +ExecCGI
        Order allow,deny
        Allow from all
    </Location>
    <Directory "/home/vagrant/build/public">
        AllowOverride None
        Options +ExecCGI

        Order allow,deny
        Allow from all
        Require all granted
        Satisfy Any
    </Directory>

    ErrorLog ${APACHE_LOG_DIR}/error.log

    # Possible values include: debug, info, notice, warn, error, crit,
    # alert, emerg.
    LogLevel debug

    CustomLog ${APACHE_LOG_DIR}/access.log combined
</VirtualHost>
NESTED
sudo rm /etc/apache2/sites-enabled/*
sudo ln -s ../sites-available/insurlt /etc/apache2/sites-enabled/000-insurlt
sudo service apache2 restart
SCRIPT

  config.vm.network "forwarded_port", guest: 80, host:8000
end
