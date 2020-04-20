# insurlt: An offensive URL-lengthener service

[![Build Status](https://github.com/remram44/insurlt/workflows/Test/badge.svg)](https://github.com/remram44/insurlt/actions) [![Say Thanks!](https://img.shields.io/badge/Say%20Thanks-!-1EAEDB.svg)](https://saythanks.io/to/remram44)

# Introduction

This is a webapp that creates redirects to user-supplied URLs, the same way the
more common URL-shortener services do (bit.ly, goo.gl, t.co). Except it makes
them very long, and full of obscenities.

It is written in C++, the obvious tool for the job.

This service used to exist as clique-salope.com, however it looks like it has
now been discontinued. Being such an important piece of the modern Internet,
someone had to put it back online.

# How to setup

You will need CMake, SQLite3, the fastcgi development kit, and a C++11
compiler like GCC >=4.6 or Clang >=2.9.

Simply configure your web server to point to the insurlt.fcgi binary. You will
need to update the source to match your domain name and favorite insults.

# Check it out

http://clique-salope.ovh/ (original French version)
http://click-bitch.ovh/ (English translation)
