#!/usr/bin/env perl
#
###########################################################################
# frameworkdeployqt.pl  v.1.0  - a script for deploying frameworks        #
#                                that are using Qt libs                   #
#                                                                         #
# Copyright (C) 2009  Dominik Kapusta     <d@ayoy.net>                    #
#                                                                         #
# This program is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by    #
# the Free Software Foundation, either version 3 of the License, or       #
# (at your option) any later version.                                     #
#                                                                         #
# This program is distributed in the hope that it will be useful,         #
# but WITHOUT ANY WARRANTY; without even the implied warranty of          #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
# GNU General Public License for more details.                            #
#                                                                         #
# You should have received a copy of the GNU General Public License       #
# along with this program.  If not, see <http://www.gnu.org/licenses/>.   #
###########################################################################
#
# Script for deploying non-Qt frameworks on MacOS X. Support for macdeployqt
# tool that provides an application bundle with required Qt frameworks and
# adjusts their id's and other frameworks' references, but does not support
# custom/third-party frameworks bundled with application.
#
# This script looks for non-Qt frameworks inside the bundle (i.e. in
# <bundle.app>/Contents/Frameworks) and replaces their references to Qt
# frameworks to point to the ones bundled with application.
# 
# This script assumes that the framework called $NAME contains a symlink to
# its current version located under $NAME.framework/$NAME. If your framework
# doesn't contain this symlink, please provide one or change two lines
# marked #ADJUST_IF_NEEDED.

use strict;

unless (@ARGV) {
  print "frameworkdeployqt v.1.0 Copyright (C) 2009 Dominik Kapusta <d\@ayoy.net>\n\n";
  print "  Usage: $0 <bundle>\n\n";
  exit;
}

# check if the argument is a bundle
my $bundle = $ARGV[0];
die "$bundle is not a bundle: $!" unless chdir "$bundle/Contents/Frameworks";

# read frameworks from the bundle
opendir DIR, ".";
my @frameworks = readdir DIR;
closedir DIR;

# select non-Qt frameworks (Qt frameworks are dealt with using macdeployqt)
@frameworks = grep /^[^Q].*\.framework/, @frameworks;

unless (@frameworks) {
  print "No non-Qt frameworks found inside $bundle bundle.\n";
  exit;
}
print "The following non-Qt frameworks were found inside $bundle bundle:\n";
# extract frameworks' names
foreach (@frameworks) {
  s/\.framework//;
  print "\t$_\n";
}

my @otool;
my @qt_frameworks;
foreach my $framework (@frameworks) {
  # get the output from otool
  @otool = `otool -L $framework.framework/$framework`; #ADJUST_IF_NEEDED
  # select Qt frameworks form otool output
  @qt_frameworks = grep /.+?Qt.+\.framework/, @otool;
  if (@qt_frameworks) {
    print "In $framework:\n";
    foreach (@qt_frameworks) {
      chomp;
      # extract Qt framework path
      s/(.+?)(Qt[^ ]+).+/$2/;
      # check if the Qt framework reference is already replaced
      if ($1 =~ /\@executable_path/) {
        print "\tfound $_\n" .
              "\t\talready replaced with \@executable_path-dependent path.\n";
        next;
      }
      # change Qt framework reference in the given framework
      system( "install_name_tool " .
              "-change $_ \@executable_path/../Frameworks/$_ " .
              "$framework.framework/$framework" ); #ADJUST_IF_NEEDED
      if ($? == 0) {
        print "\tfound and replaced $_ :-)\n";
      } else {
        print "\tfound $framework - replacing failed :-(\n";
      }
    }
  } else {
    print "No Qt frameworks are referenced by $framework.\n"
  }
}
