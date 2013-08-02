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

my $pat = qr,\x22,;
my ($ant, $ant_home, $java_home, $java_mode) =
	map { s,$pat,\\\x22,g; $_; } (@ARGV);
$fh->print($Config{startperl}, "\n");

$fh->print(<<EOF);

#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Apache Ant wrapper.
##

\$ENV{ANT_HOME} = "$ant_home";
\$ENV{JAVA_HOME} = "$java_home";
my \$java_mode = "$java_mode";

EOF

$fh->print(<<'EOF');
my $maxheap = $ENV{ANT_MAXHEAP};
my $opts = $ENV{ANT_OPTS};
$opts = ($opts) ? "$opts $java_mode" : $java_mode;
if ($maxheap) {
	my $xopts = "-Xmx$maxheap";
	$opts .= " $xopts";
}
$ENV{ANT_OPTS} = $opts;

umask(022);

EOF

$fh->print(<<EOF);
exec("$ant", \@ARGV);
EOF
