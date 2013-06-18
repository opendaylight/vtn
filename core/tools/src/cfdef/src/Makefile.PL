# -*- Mode: perl; -*-

#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Generate makefiles for cfdef package.
##

use strict;
use 5.008;
use vars qw(%Config $NAME $VERSION $NOSCANDIR);

use Config;
use DirHandle;
use ExtUtils::MakeMaker;

$NAME = 'PFC::Conf';
$NOSCANDIR = qr,/(CVS|RCS|\.(git|svn))/,;

sub MY::libscan
{
	package MY;

	my ($me, $path) = @_;
	return undef if ($path =~ m,$main::NOSCANDIR,);
	return $path;
}

MAIN:
{
	my (%plfiles, @exe);
	my $dirp = DirHandle->new('bin') or die "opendir(bin) failed: $!\n";
	my $regexp = qr,^(.+)\.PL$,;
	while (my $dp = $dirp->read()) {
		if ($dp =~ /$regexp/) {
			my $name = $1;
			my $path = 'bin/' . $name;
			my $pl = $path . '.PL';
			$plfiles{$pl} = $path;
			push(@exe, $path);
		}
	}

	my (%args) = (NAME => $NAME, VERSION_FROM => 'lib/PFC/Conf.pm',
		      PL_FILES => \%plfiles, EXE_FILES => \@exe,
		      clean => {FILES => join(' ', @exe)});

	# Accept configuration via command line option.
	while (my $arg = shift(@ARGV)) {
		my ($key, $value) = split(/=/, $arg, 2);
		$args{$key} = $value;
	}

	my $basedir = $args{BASEDIR};
	if ($basedir) {
		my $arch = $Config{archname};
		my $libdir = $basedir . '/lib';
		my $archlibdir = $libdir . '/' . $arch;
		my $bindir = $basedir . '/bin';
		my $mandir = $basedir . '/man';
		my $man1dir = $mandir . '/man1';
		my $man3dir = $mandir . '/man3';

		delete($args{BASEDIR});
		$args{INSTALLSITELIB} = $libdir;
		$args{INSTALLARCHLIB} = $archlibdir;
		$args{INSTALLSITEARCH} = $archlibdir;
		$args{INSTALLSCRIPT} = $bindir;
		$args{INSTALLSITEBIN} = $bindir;
		$args{INSTALLSITEMAN1DIR} = $man1dir;
		$args{INSTALLSITEMAN3DIR} = $man3dir;

		use ExtUtils::MM;
		my $mm = ExtUtils::MM->new({NAME => $NAME});
		my (@ver) = $mm->installvars();
		foreach my $v (@ver) {
			if ($v eq 'SITESCRIPT') {
				$args{INSTALLSITESCRIPT} = $bindir;
				last;
			}
		}
	}

	# Generate makefile.
	WriteMakefile(%args);
}
