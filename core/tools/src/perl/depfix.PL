use strict;
use vars qw(%Config);
use Config;
use FileHandle;
use File::Basename;
use POSIX;

my $fname = $0;
$fname =~ s,\.PL$,,;

my $fh = FileHandle->new($fname, O_CREAT|O_TRUNC|O_WRONLY, 0755) or
	die "open($fname) failed: $!\n";

$fh->print($Config{startperl}, "\n");

$fh->print(<<'EOF');

#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Fix up dependency file.
##

use strict;
use FileHandle;
use POSIX;

MAIN:
{
	die "Usage: depfix depfile\n" unless (@ARGV == 1);
	my $file = $ARGV[0];
	my $fh = FileHandle->new($file) or die "open($file) failed: $!\n";

	my (@contents) = <$fh>;

	$fh = FileHandle->new($file, O_WRONLY|O_TRUNC|O_CREAT, 0644) or
		die "open($file) failed: $!\n";

	my $pattern = qr,^([^:]+\.o):,;

	foreach my $line (@contents) {
		$line =~ s,$pattern,\$(OBJDIR)/$1:,g;
		$fh->print($line);
	}
}
EOF
