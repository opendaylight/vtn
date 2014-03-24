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

my $bldstamp = time();

$fh->print($Config{startperl}, "\n");

$fh->print(<<'EOF');

#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Create Apache Ant project file to build Java library.
##

use strict;
use vars qw($PROGNAME $VERBOSE $CURDIR $OUTPUT $PVERSION $OBJDIR $JARDIR
	    $ENCODING $CMDLINEFILE);
use Cwd qw(abs_path);
use DirHandle;
use File::Basename;
use File::Path;
use FileHandle;
use Getopt::Long;
use POSIX;

EOF
$fh->print(<<EOF);
use constant	BUILD_STAMP	=> $bldstamp;
EOF

$fh->print(<<'EOF');

$PROGNAME = 'mkjproject';

$OUTPUT = 'build.xml';
$PVERSION = '1.0';
$OBJDIR = 'objs';
$JARDIR = 'jars';
$ENCODING = 'utf-8';

use constant	PACKAGE_VENDOR		=> 'OpenDaylight';
use constant	PACKAGE_VENDOR_ID	=> 'org.opendaylight';
use constant	PACKAGE_FILTER		=> 'org/opendaylight/vtn/**';
use constant	DIR_EXTLIB		=> 'ext';
use constant	DIR_CLASSDIR		=> 'classdir';

use constant	PROP_TESTNAME		=> 'pkg.test.name';
use constant	PROP_JARINDEX		=> 'pkg.jar.index';
use constant	PROP_WEBAPP_BASE	=> 'pkg.webapp.base';

use constant	TARGET_JAR		=> 'jar';
use constant	TARGET_BUILD		=> 'build';
use constant	TARGET_RESOURCE		=> 'resource';
use constant	TARGET_WAR		=> 'war';
use constant	TARGET_INSTALL_WEBAPP	=> 'install-webapp';
use constant	TARGET_INSTALL_WAR	=> 'install-war';
use constant	TARGET_DOC		=> 'doc';

use constant	META_INF		=> 'META-INF';
use constant	MANIFEST_MF		=> META_INF . '/MANIFEST.MF';
use constant	WEB_INF			=> 'WEB-INF';
use constant	WEB_XML			=> WEB_INF . '/web.xml';

use constant	SYSPROP_CORE_LIBPATH	=> 'pflow.core.libpath';

use constant	PATTERN_PROP		=> '**/*.properties';
use constant	FILTER_TOKEN_CHAR	=> '%';

sub usage($);
sub fatal(@);
sub stringify($);
sub dir_create($$);
sub get_project_name();
sub get_project_version($);
sub get_abspath($);
sub read_srclist($\@\@);
sub uniq_list(\@);
sub check_cmdline(\@);
sub check_srcdir(\@);
sub check_classpath(\@);
sub check_build_classpath(\@);
sub check_extlibs(\@);
sub check_webapp($);
sub create_javac($\@$%);
sub create_resource($\@$$%);
sub create_jar($$$\%%);
sub create_war($$$$$\%%);
sub create_install_webapp($$$$);
sub create_install_war($$$$$);
sub create_javadoc($\@$%);
sub create_junit($$$%);
sub create_classpath($$%);

END {
	if ($? != 0) {
		rmtree($OUTPUT) if (-f $OUTPUT);
		rmtree($CMDLINEFILE) if ($CMDLINEFILE);
	}
}

# File which keeps command-line arguments.
{
	package CommandLineFile;

	use overload	'""'	=> \&getName;

	use Fcntl qw(:flock);
	use FileHandle;
	use POSIX;

	use constant	FILENAME	=> '.mkjproject';

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;

		my $name = $main::OBJDIR . '/' . FILENAME;
		my $fh = FileHandle->new($name, O_RDWR | O_CREAT, 0644);
		&main::fatal("open($name) failed: $!") unless ($fh);
		$main::CMDLINEFILE = $name;

		flock($fh, LOCK_EX) or
			&main::fatal("$name: flock() failed: $!");

		my $me = {HANDLE => $fh, NAME => $name};

		return bless($me, $class);
	}

	sub getName
	{
		my $me = shift;

		return $me->{NAME};
	}

	sub check
	{
		my $me = shift;
		my ($cmdline) = @_;

		my $fh = $me->{HANDLE};
		$fh->seek(0, SEEK_SET) or $me->fatal("seek(0) failed: $!");

		# The first line is build time of mkjproject.
		my $stamp = $fh->getline();
		return 1 unless (defined($stamp) and
				 $stamp == main::BUILD_STAMP());

		# The second line is command line arguments.
		my $saved = $fh->getline();
		return 1 unless (defined($saved));
		chomp($saved);

		return 1 if ($cmdline ne $saved);

		# The third line is current directory path.
		my $curdir = $fh->getline();
		return 1 unless (defined($curdir));
		chomp($curdir);

		return ($curdir ne $main::CURDIR);
	}

	sub save
	{
		my $me = shift;
		my ($cmdline) = @_;

		my $fh = $me->{HANDLE};

		$fh->truncate(0) or $me->fatal("truncate(0) failed: $!");
		$fh->seek(0, SEEK_SET) or $me->fatal("seek(0) failed: $!");

		$fh->print(main::BUILD_STAMP(), "\n");
		$fh->print($cmdline, "\n");
		$fh->print($main::CURDIR, "\n");
        }

	sub fatal
	{
		my $me = shift;

		my $name = $me->getName();
		&main::fatal($name, ': ', @_);
	}
}

# Base class of XML node.
{
	package XmlNode;

	use overload	'""'	=> \&getId;

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($name, %attr) = @_;

		my $me = {NAME => $name, ATTR => \%attr, CHILDREN => []};

		return bless($me, $class);
	}

	sub getId
	{
		my $me = shift;

		my $attr = $me->{ATTR};

		foreach my $k (qw(name id)) {
			my $name = $attr->{$k};
			return $name if ($name);
		}

		&main::fatal($me->{NAME}, ": No identifier");
	}

	sub getName
	{
		my $me = shift;

		return $me->{NAME};
	}

	sub add
	{
		my $me = shift;
		my ($node) = @_;

		push(@{$me->{CHILDREN}}, $node);
	}

	sub addNode
	{
		my $me = shift;
		my ($name, %attr) = @_;

		my $node = XmlNode->new($name, %attr);
		$me->add($node);

		return $node;
	}

	sub addAttribute
	{
		my $me = shift;
		my ($key, $value) = @_;

		$me->{ATTR}->{$key} = $value;
	}

	sub getAttribute
	{
		my $me = shift;
		my ($key) = @_;

		return $me->{ATTR}->{$key};
	}

	sub attrToString
	{
		my $me = shift;

		my $attr = $me->{ATTR};
		my $ret;
		foreach my $k (sort(keys(%$attr))) {
			my $v = &main::stringify($attr->{$k});
			my $a = ' ' . $k . '=' . $v;
			$ret .= $a;
		}

		return $ret;
	}

	sub output
	{
		my $me = shift;
		my ($out, $depth) = @_;

		my $indent = ' ' x $depth;
		my $name = $me->getName();
		my $attr = $me->attrToString();

		$out->print($indent, '<', $name, $attr);
		my $children = $me->{CHILDREN};
		unless (@$children) {
			$out->print(" />\n");
			return;
		}

		$out->print(">\n");
		$depth += 2;
		foreach my $node (@$children) {
			$node->output($out, $depth);
		}

		$out->print($indent, '</', $name, ">\n");
	}
}

# Ant property.
{
	package AntProperty;

	use base qw(XmlNode);

	use overload	'""'	=> \&refer;

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($name, $value) = @_;

		my $me = $class->SUPER::new('property', name => $name,
					    value => $value);

		return bless($me, $class);
	}

	sub refer
	{
		my $me = shift;

		my $name = $me->getAttribute('name');

		return dereference($name);
	}

	# Static method.
	sub dereference
	{
		my ($name) = @_;

		return '${' . $name . '}';
	}
}

# Ant project.
{
	package AntProject;

	use base qw(XmlNode);

	sub new
	{
		my $this = shift;
		my $class = ref($this) || $this;
		my ($name, $default, $basedir) = @_;

		my $me = $class->SUPER::new('project', name => $name,
					    default => $default,
					    basedir => $basedir);

		return bless($me, $class);
	}

	sub output
	{
		my $me = shift;
		my ($out) = @_;

		$out->print(<<OUT);
<?xml version="1.0" encoding="utf-8"?>

OUT

		$me->SUPER::output($out, 0);
	}

	sub addProperty
	{
		my $me = shift;
		my ($name, $value) = @_;

		my $prop = AntProperty->new($name, $value);
		$me->add($prop);

		return $prop;
	}

	sub addTarget
	{
		my $me = shift;
		my ($name, $desc, @depends) = @_;

		my (%attr) = (name => $name, description => $desc);
		$attr{depends} = join(', ', @depends) if (@depends);

		return $me->addNode('target', %attr);
	}
}

MAIN:
{
	Getopt::Long::Configure(qw(no_ignore_case require_order));

	my $terminate = sub { exit(3); };
	$SIG{INT} = $terminate;
	$SIG{TERM} = $terminate;

	my $deftarget = TARGET_JAR;
	my $javadoc;
	my $javadoc_func = sub {
		$javadoc = 1;
		$deftarget = TARGET_DOC;
	};
	my (@orgargv) = (@ARGV);
	my $help = sub { usage(0); };
	my (@srcdir, $name, $title, $spec_title, $debug, @classpath, @extlibs);
	my (@excludes, @jcargs, @jnidir, $main_class, $junit, $srclist);
	my ($webapp, $webapp_base, $nocp, @build_cp);
	my (@filter_pattern) = (PATTERN_PROP);

	my (%filters, $token);
	my $filter_func = sub {
		my ($key, $value) = @_;

		if ($token) {
			$filters{$token} = $value;
			undef $token;
		}
		else {
			$token = $value;
		}
	};

	usage(1) unless (GetOptions
			 ('help'		=> $help,
			  's|srcdir=s'		=> \@srcdir,
			  'srclist=s'		=> \$srclist,
			  'o|output=s'		=> \$OUTPUT,
			  'n|name=s'		=> \$name,
			  't|title=s'		=> \$title,
			  'T|spec-title=s'	=> \$spec_title,
			  'V|project-version=s'	=> \$PVERSION,
			  'O|objdir=s'		=> \$OBJDIR,
			  'J|jardir=s'		=> \$JARDIR,
			  'g|debug'		=> \$debug,
			  'e|encoding=s'	=> \$ENCODING,
			  'c|classpath=s'	=> \@classpath,
			  'C|build-classpath=s'	=> \@build_cp,
			  'E|external-libs=s'	=> \@extlibs,
			  'x|excludes=s',	=> \@excludes,
			  'X|javac-args=s'	=> \@jcargs,
			  'm|main-class=s'	=> \$main_class,
			  'u|junit=s'		=> \$junit,
			  'w|webapp=s'		=> \$webapp,
			  'W|webapp-base=s'	=> \$webapp_base,
			  'f|filter=s{2}'	=> $filter_func,
			  'F|filter-pattern=s'	=> \@filter_pattern,
			  'no-classpath'	=> \$nocp,
			  'jnidir=s'		=> \@jnidir,
			  'javadoc'		=> $javadoc_func,
			  'v|verbose+'		=> \$VERBOSE));

	$CURDIR= getcwd();
	fatal("Unable to determine current directory: $!") unless ($CURDIR);

	$OBJDIR = get_abspath($OBJDIR);
	$JARDIR = get_abspath($JARDIR);
	read_srclist($srclist, @srcdir, @orgargv) if ($srclist);
	check_cmdline(@orgargv);
	undef @orgargv;

	$name = get_project_name() unless ($name);
	$title = $name unless ($title);
	$spec_title = $name unless ($spec_title);
	my ($vmajor, $vminor, $vrev, $vpatch) = get_project_version($PVERSION);

	if ($VERBOSE) {
		print "Project Name         : $name\n";
		print "Project Version      : $PVERSION\n";
		print "Object Directory     : $OBJDIR\n";
		print "JAR Directory        : $JARDIR\n";
		print "Source Directory     : ", join(' ', @srcdir), "\n";
		print "Class Path           : ", join(' ', @classpath), "\n";
		print "External Libraries   : ", join(' ', @extlibs), "\n";
		print "Compiler Option      : ", join(' ', @jcargs), "\n";
		print "Web Application Root : $webapp\n" if ($webapp);
	}

	print "=== Generating $OUTPUT\n";

	check_srcdir(@srcdir);
	check_classpath(@classpath);
	check_build_classpath(@build_cp);
	check_extlibs(@extlibs);
	$webapp = check_webapp($webapp) if ($webapp);
	uniq_list(@filter_pattern);

	if ($webapp_base) {
		fatal("-W: An absolute path must be specified: $webapp_base")
			unless ($webapp_base =~ m,^/,);
	}

	# Create Ant project.
	my $proj = AntProject->new($name, $deftarget, $CURDIR);

	# Set properties.
	$proj->addProperty('pkg.version', $PVERSION);
	my (@pver);
	{
		my $pvmajor = $proj->addProperty('pkg.version.major', $vmajor);
		my $pvminor = $proj->addProperty('pkg.version.minor', $vminor);
		push(@pver, $pvmajor, $pvminor);

		if (defined($vrev)) {
			my $pvrev = $proj->addProperty('pkg.version.revision',
						       $vrev);
			push(@pver, $vrev);
		}
		if (defined($vpatch)) {
			my $pvpatch = $proj->
				addProperty('pkg.version.patchlevel', $vpatch);
			push(@pver, $pvpatch);
		}
	}

	my $pjarfile = $proj->addProperty('pkg.jar.name',
					  '${ant.project.name}.jar');
	my $pdocdir = $proj->addProperty('pkg.docdir',
					 join('-', '${ant.project.name}',
					      join('_', @pver), 'docs'));
	my $pobjdir = $proj->addProperty('pkg.objdir', $OBJDIR);
	my $pclassdir = $proj->addProperty('pkg.classdir', $pobjdir . '/' .
					   DIR_CLASSDIR);
	my $pjardir = $proj->addProperty('pkg.jar.objdir', $JARDIR);
	my $pjarpath = $proj->addProperty('pkg.jar.path',
					  $pjardir . '/' . $pjarfile);
	my $pdocpath = $proj->addProperty('pkg.docpath',
					  '${basedir}/' . $pdocdir);

	my ($pwebroot, $pwebxml, $pwarname, $pwarpath);
	if ($webapp) {
		$pwarname = $proj->addProperty('pkg.war.name',
					       '${ant.project.name}.war');
		$pwebroot = $proj->addProperty('pkg.war.root', $webapp);
		$pwebxml = $proj->
			addProperty('pkg.war.webxml',
				    '${pkg.war.root}/' . WEB_XML);
		$pwarpath = $proj->
			addProperty('pkg.war.path',
				    '${pkg.objdir}/${pkg.war.name}');
	}

	my ($pwebbase, $pwebdir, $pwebwar);
	if ($webapp_base) {
		$pwebbase = $proj->addProperty(PROP_WEBAPP_BASE, $webapp_base);

		my $webdir = $pwebbase . '/' . '${ant.project.name}';
		$pwebdir = $proj->addProperty("pkg.webapp.dir", $webdir);

		my $webwar = $pwebbase . '/' . $pwarname;
		$pwebwar = $proj->addProperty("pkg.webapp.war", $webwar);
	}

	my $patsrc = $proj->addNode('patternset', id => 'pkg.pat.src');
	$patsrc->addNode('include', name => PACKAGE_FILTER . '/*.java');

	my $patres = $proj->addNode('patternset', id => 'pkg.pat.resource');
	$patres->addNode('exclude', name => '**/*.java');
	$patres->addNode('exclude', name => '**/package.html');
	$patres->addNode('exclude', name => '**/.svn/**');
	$patres->addNode('exclude', name => '**/*~');
	foreach my $exc (@excludes) {
		$patres->addNode('exclude', name => $exc);
	}

	my $patprop;
	if (%filters) {
		$patprop = $proj->addNode('patternset', id => 'pkg.pat.prop');
		foreach my $pat (@filter_pattern) {
			$patres->addNode('exclude', name => $pat);
			$patprop->addNode('include', name => $pat);
		}
		foreach my $exc (@excludes) {
			$patprop->addNode('exclude', name => $exc);
		}
	}

	my ($pclasspath, @jarclpath);
	if (@classpath or @extlibs or @build_cp) {
		$pclasspath = create_classpath($proj, 'pkg.build.classpath',
					       classpath => \@classpath,
					       build_classpath => \@build_cp,
					       extlibs => \@extlibs,
					       jardir => $pjardir);
		(@jarclpath) = (@classpath, @extlibs) unless ($nocp);
	}

	unless ($javadoc) {
		# Create javac task.
		my (%javac_args) = (jcargs => \@jcargs, debug => $debug,
				    classpath => $pclasspath,
				    pattern => $patsrc);
		create_javac($proj, @srcdir, $pclassdir, %javac_args);

		# Create task to copy resource files.
		create_resource($proj, @srcdir, $pclassdir, $patres,
				filters => \%filters,
				pattern_prop => $patprop);

		# Create attributes for manifest.
		my %mattr;
		$mattr{'Implementation-Title'} = $title;
		$mattr{'Implementation-Version'} = $PVERSION;
		$mattr{'Specification-Title'} = $spec_title;
		$mattr{'Specification-Version'} = $vmajor . '.' . $vminor;
		$mattr{'Main-Class'} = $main_class if (defined($main_class));

		# Create task to build JAR file.
		create_jar($proj, $pjarpath, $pclassdir, %mattr,
			   classpath => $pclasspath,
			   manifest_classpath => \@jarclpath);

		if ($webapp) {

			# Create task to build WAR file.
			my $has_meta = (-d $webapp . '/' . META_INF);
			create_war($proj, $pwebxml, $pwebroot, $pwarpath,
				   $pclassdir, %mattr, has_meta => $has_meta);

			# Create task to install web application.
			create_install_webapp($proj, $pwarpath, $pwebdir,
					      $pwebwar);

			# Create task to install web application as WAR file.
			create_install_war($proj, $pwarpath, $pwebbase,
					   $pwebdir, $pwebwar);
		}
	}

	# Create task to build javadoc.
	create_javadoc($proj, @srcdir, $pdocpath, pattern => $patsrc,
		       classpath => $pclasspath);

	unless ($javadoc) {
		if ($junit) {
			# Enable test listener events.
			$proj->addProperty
				('ant.junit.enabletestlistenerevents', 'on');

			# Create task to run JUnit tests.
			create_junit($proj, $junit, $pjarpath,
				     jnidir => \@jnidir);
		}
	}

	my $out = FileHandle->new($OUTPUT, O_CREAT | O_TRUNC | O_WRONLY, 0644);
	fatal("Unable to create output file: $OUTPUT: $!") unless ($out);

	$proj->output($out);

	# Clean up old classdir.
	my $dir = $OBJDIR . '/' . DIR_CLASSDIR;
	rmtree($dir) if (-d $dir);
}

sub usage($)
{
	my ($status) = @_;

	my $out = ($status) ? \*STDERR : \*STDOUT;
	$out->print(<<OUT);
Usage: $PROGNAME [options] [<base-directory>]

OUT
	if ($status) {
		$out->print(<<OUT);
Try "$PROGNAME --help".
OUT
	}
	else {
		my $testname = PROP_TESTNAME;
		my $target_doc = TARGET_DOC;
		my $target_war = TARGET_WAR;
		my $webxml = WEB_XML;
		my $webapp_base = PROP_WEBAPP_BASE;
		my $token_char = FILTER_TOKEN_CHAR;
		my $pattern_prop = PATTERN_PROP;

		$out->print(<<OUT);
        Create Apache Ant project file to build Java library.

Options:
   -s|--srcdir <directory>
        Specify source directory path which contains Java source files.
        Multiple directories can be specified.

   --srclist <file>
        Append all lines in the specified file to source directory list.

   -o|--output <file>
        Specify file path to output project file.
        Default is "$OUTPUT".

   -n|--name <name>
        Specify name of the project.
        If omitted, it is derived from the name of the current directory.

   -t|--title <title>
        Specify implementation title of this project.
        If omitted, the project name is used.

   -T|--spec-title <title>
        Specify specification title of this project.
        If omitted, the project name is used.

   -V|--project-version <version>
        Specify version number of the project.
        Default is "$PVERSION".

   -O|--objdir <directory>
        Specify directory path to keep generated files.
        Default is "$OBJDIR".

   -J|--jardir <directory>
        Specify directory path to put JAR file.
        Default is "$JARDIR".

   -g|--debug
        Enable debugging switch on javac.

   -e|--encoding <encoding>
        Specify encoding of Java source files.
        Default is "$ENCODING".

   -c|--classpath <file>
        Specify JAR file to be added to classpath.
        Note that this value must be relative to JAR file output directory
        specified by '-J'.
        Multiple arguments can be specified.

   -C|--build-classpath <file>
        Specify JAR file to be added to classpath only on build.
	JAR files specified by this option are never put into "Class-Path"
	in MANIFEST.MF.
        Multiple arguments can be specified.

   -E|--external-libs <file>
        Specify external JAR file to be added to classpath.
        Multiple arguments can be specified.

   -x|--exclude <pattern>
        Specify Ant file pattern that matches files to be eliminated from
        JAR file.

   -X|--javac-args <javac-arg>
        Specify additional command-line arguments for javac.
        Multiple arguments can be specified.

   -m|--main-class <class-name>
        Specify main class name for standalone application.

   -u|--junit <target>
        Define task to run JUnit test.
        <target> is the name of the JUnit task.
        The class name of the JUnit must be specified by the system property
        named "$testname".

   -w|--webapp <directory>
        Specify root directory of web application to be installed.
        If this option is specified, "$target_war" target, which generates
        WAR file for Web application, is added to build.xml.
        Note that "$webxml" must exist under the specified directory.

   -W|--webapp-base <directory>
        Specify installation directory of web application.
        If not specified, installation directory must be specified by
        the system property named "$webapp_base".

   -f|--filter <token> <value>
        Specify token filter applied to resource file copy.
        If at least one filter is specified, resource files are copied
        replacing all the occurrences of the token "${token_char}<token>${token_char}" with "<value>".

   -F|--filter-pattern <pattern>
        Specify Ant file pattern that matches text files to apply token
        filters specified by -f option. Note that "$pattern_prop" is
        always prepended to filter patterns.

   --no-classpath
        Do not set "Class-Path" to MANIFEST.MF.

   --jnidir <directory>
        Specify directory path which contains JNI files.
        Multiple arguments can be specified.
        This parameter is used to pass library search path to JUnit tests.

   --javadoc
        Create only "$target_doc" target, which generates code documentation.

   -v|--verbose
        Verbose mode.
OUT
	}

	exit $status;
}

sub fatal(@)
{
	die "*** ERROR: ", join('', @_), "\n";
}

sub stringify($)
{
	my ($path) = @_;

	$path =~ s,\x22,\\\x22,g;

	return '"' . $path . '"';
}

sub dir_create($$)
{
	my ($path, $mode) = @_;

	eval { mkpath($path, undef, $mode); };
	if ($@) {
		my $err = "$@";

		chomp($err);
		fatal("Unable to create directory: $path: $err");
	}
}

sub get_project_name()
{
	my $cwd = $CURDIR;
	$cwd =~ s,/+$,,g;

	return basename($cwd);
}

sub get_project_version($)
{
	my ($pver) = @_;

	if ($pver =~ m,^(\d+)\.(\d+)(\.(\d+)(\.(\d+))?)?$,) {
		return ($1, $2, $4, $6);
	}

	fatal("Unexpected format of project version: $pver");
}

sub get_abspath($)
{
	my ($path) = @_;

	my $p = abs_path($path);
	fatal("Unable to determine absolute path to $path.") unless ($p);

	return $p;
}

# Check whether a project file must be rebuilt or not.
sub check_cmdline(\@)
{
	my ($argv) = @_;

	unless (-d $OBJDIR) {
		dir_create($OBJDIR, 0755);
	}

	my $cmdfile = CommandLineFile->new();
	my $cmdline = join(' ', @$argv);

	if (-f $OUTPUT and !$cmdfile->check($cmdline)) {
		# Command line argument is not changed.
		print "--- Project file is up to date.\n" if ($VERBOSE);

		exit 0;
	}

	# Save command line arguments.
	$cmdfile->save($cmdline);
}

sub read_srclist($\@\@)
{
	my ($srclist, $srcdir, $argv) = @_;

	my $fh = FileHandle->new($srclist);
	fatal("Unable to open srclist file: $srclist: $!") unless ($fh);

	my $empty = qr,^\s*$,;
	while (my $line = $fh->getline()) {
		next if ($line =~ $empty);
		chomp($line);
		push(@$srcdir, $line);
		push(@$argv, '-s', $line);
	}
}

sub uniq_list(\@)
{
	my ($list) = @_;

	my (@uniq, %map);
	foreach my $arg (@$list) {
		unless ($map{$arg}) {
			$map{$arg} = 1;
			push(@uniq, $arg);
		}
	}

	(@$list) = (@uniq);
}


sub check_srcdir(\@)
{
	my ($srcdir) = @_;

	fatal("At least one source directory must be specified.")
		unless (@$srcdir);

	my (@s);
	foreach my $d (@$srcdir) {
		fatal("Source directory does not exist: $d") unless (-d $d);
		push(@s, get_abspath($d));
	}

	(@$srcdir) = (@s);
}

sub check_classpath(\@)
{
	my ($classpath) = @_;

	my $pat = qr,/,;
	my (@libs, %fmap);
	foreach my $f (@$classpath) {
		next if ($fmap{$f});
		$fmap{$f} = 1;
		fatal("--classpath: JAR file name must be specified: $f")
			if ($f =~ $pat);
		my $path = $JARDIR . '/' . $f;
		fatal("--classpath: JAR file does not exist: $f")
			unless (-f $path);
		push(@libs, $f);
	}

	(@$classpath) = (@libs);
}

sub check_build_classpath(\@)
{
	my ($classpath) = @_;

	my (@libs, %fmap);
	foreach my $f (@$classpath) {
		fatal("--build-classpath: JAR file does not exist: $f")
			unless (-f $f);
		my $path = get_abspath($f);
		next if ($fmap{$path});
		$fmap{$path} = 1;
		push(@libs, $path);
	}

	(@$classpath) = (@libs);
}

sub check_extlibs(\@)
{
	my ($extlibs) = @_;

	return [] unless (@$extlibs);

	my $pat = qr,/,;
	my (@jextlibs, %fmap);
	foreach my $f (@$extlibs) {
		next if ($fmap{$f});
		$fmap{$f} = 1;
		fatal("--external-libs: JAR file name must be specified: $f")
			if ($f =~ $pat);
		my $fname = DIR_EXTLIB . '/' . $f;
		my $path = $JARDIR . '/' . $fname;
		fatal("--external-libs: JAR file does not exist: $f")
			unless (-f $path);
		push(@jextlibs, $fname);
	}

	(@$extlibs) = (@jextlibs);
}

sub check_webapp($)
{
	my ($webapp) = @_;

	fatal("-w: Web application root directory does not exist: $webapp")
		unless (-d $webapp);

	my $webinf = $webapp . '/' . WEB_INF;
	fatal('-w: ', WEB_INF, ' directory does not exist under "', $webapp,
	      '".') unless (-d $webinf);
	my $webxml = $webapp . '/' . WEB_XML;
	fatal('-w: ', WEB_XML, ' does not exist under "', $webapp, '".')
		unless (-f $webxml);
	my $classes = $webinf . '/classes';
	fatal('-w: "classes" must not exist under "', $webinf, '".')
		if (-x $classes);

	my $metainf = $webapp . '/' . META_INF;
	fatal('-w: ', META_INF, ' must be a directory.')
		if (-x $metainf and ! -d $metainf);
	my $manifest = $webapp . '/' . MANIFEST_MF;
	fatal('-w: ', MANIFEST_MF, ' must not exist under "', $webapp, '".')
		if (-f $manifest);

	return get_abspath($webapp);
}

sub create_javac($\@$%)
{
	my ($proj, $srcdir, $destdir, %args) = @_;

	my $name = $args{name} || TARGET_BUILD;
	my $desc = $args{desc} || 'Compile all Java source files.';
	my $depends = $args{depends} || [];
	my $tgt = $proj->addTarget($name, $desc, @$depends);
	$tgt->addNode('mkdir', dir => $destdir);

	my (%attr) = (includeAntRuntime => 'off', encoding => $ENCODING,
		      destdir => $destdir);
	$attr{debug} = 'on' if ($args{debug});
	my $classpath = $args{classpath};
	$attr{classpathref} = $classpath if ($classpath);
	my $javac = $tgt->addNode('javac', %attr);

	foreach my $src (@$srcdir) {
		$javac->addNode('src', path => $src);
	}

	my $pattern = $args{pattern};
	if ($pattern) {
		$javac->addNode('patternset', refid => $pattern);
	}

	foreach my $arg (@{$args{jcargs}}) {
		$javac->addNode('compilerarg', value => $arg);
	}
}

sub create_resource($\@$$%)
{
	my ($proj, $srcdir, $destdir, $pattern, %args) = @_;

	my $name = $args{name} || TARGET_RESOURCE;
	my $desc = $args{desc} || 'Copy resources to build directory.';
	my $tgt = $proj->addTarget($name, $desc);
	$tgt->addNode('mkdir', dir => $destdir);
	my $cp = $tgt->addNode('copy', todir => $destdir,
			       preservelastmodified => 'on');
	foreach my $src (@$srcdir) {
		my $fset = $cp->addNode('fileset', dir => $src);
		$fset->addNode('patternset', refid => $pattern);
	}

	my $filters = $args{filters};
	my $patprop = $args{pattern_prop};
	if ($filters and %$filters and $patprop) {
		$cp = $tgt->addNode('copy', todir => $destdir,
				    preservelastmodified => 'on');
		foreach my $src (@$srcdir) {
			my $fset = $cp->addNode('fileset', dir => $src);
			$fset->addNode('patternset', refid => $patprop);
		}

		my $fset = $cp->addNode('filterset',
					begintoken => FILTER_TOKEN_CHAR,
					endtoken => FILTER_TOKEN_CHAR);
		while (my ($token, $value) = each(%$filters)) {
			$fset->addNode('filter', token => $token,
				       value => $value);
		}
	}
}

sub create_jar($$$\%%)
{
	my ($proj, $jarfile, $basedir, $mattr, %args) = @_;

	foreach my $k (qw(Implementation-Vendor Specification-Vendor)) {
		$mattr->{$k} = PACKAGE_VENDOR;
	}
	$mattr->{'Implementation-Vendor-Id'} = PACKAGE_VENDOR_ID;

	my $name = $args{name} || TARGET_JAR;
	my $depends = $args{depends} || [TARGET_BUILD, TARGET_RESOURCE];
	my $desc = $args{desc} || 'Build JAR file.';
	my $tgt = $proj->addTarget($name, $desc, @$depends);

	my $classpath = $args{manifest_classpath};
	if ($classpath and @$classpath) {
		$mattr->{'Class-Path'} = join(' ', @$classpath);
	}
	else {
		delete($mattr->{'Class-Path'});
	}

	my $index = AntProperty::dereference(PROP_JARINDEX);
	my $jar = $tgt->addNode('jar', jarfile => $jarfile, index => $index,
				basedir => $basedir);

	my $pclasspath = $args{classpath};
	$jar->addNode('indexjars', refid => $pclasspath) if ($pclasspath);

	my $manifest = $jar->addNode('manifest');
	foreach my $k (sort(keys(%$mattr))) {
		my $v = $mattr->{$k};

		$manifest->addNode('attribute', name => $k, value => $v);
	}
}

sub create_war($$$$$\%%)
{
	my ($proj, $webxml, $webroot, $warfile, $classdir, $mattr, %args) = @_;

	foreach my $k (qw(Implementation-Vendor Specification-Vendor)) {
		$mattr->{$k} = PACKAGE_VENDOR;
	}
	$mattr->{'Implementation-Vendor-Id'} = PACKAGE_VENDOR_ID;

	my $name = $args{name} || TARGET_WAR;
	my $depends = $args{depends} || [TARGET_BUILD, TARGET_RESOURCE];
	my $desc = $args{desc} || 'Build WAR file.';
	my $tgt = $proj->addTarget($name, $desc, @$depends);

	delete($mattr->{'Class-Path'});
	delete($mattr->{'Main-Class'});

	my $war = $tgt->addNode('war', webxml => $webxml, 
				destfile => $warfile);

	my $manifest = $war->addNode('manifest');
	foreach my $k (sort(keys(%$mattr))) {
		my $v = $mattr->{$k};

		$manifest->addNode('attribute', name => $k, value => $v);
	}

	# Copy Java class files to WEB-INF/classes.
	$war->addNode('classes', dir => $classdir);

	# Copy all files under application root directory except for
	# WEB-INF and META-INF.
	my $fset = $war->addNode('fileset', dir => $webroot);
	$fset->addNode('exclude', name => WEB_INF . '/**');
	$fset->addNode('exclude', name => META_INF . '/**');

	# Copy WEB-INF except for web.xml.
	my $root = $webroot->refer();
	my $dir = $root . '/' . WEB_INF;
	my $webinf = $war->addNode('webinf', dir => $dir);
	$webinf->addNode('exclude', name => WEB_XML);

	# Copy META-INF except for MANIFEST.MF if exists.
	if ($args{has_meta}) {
		$dir = $root . '/' . META_INF;
		my $metainf = $war->addNode('metainf', dir => $dir);
		$metainf->addNode('exclude', name => MANIFEST_MF());
	}
}

sub create_install_webapp($$$$)
{
	my ($proj, $warfile, $webdir, $webwar) = @_;

	my $tgt = $proj->addTarget(TARGET_INSTALL_WEBAPP,
				   "Install web application.", TARGET_WAR);
	$tgt->addNode('delete', dir => $webdir, quiet => 'true');
	$tgt->addNode('delete', file => $webwar, quiet => 'true');
	$tgt->addNode('mkdir', dir => $webdir);
	$tgt->addNode('unwar', src => $warfile, dest => $webdir);
}

sub create_install_war($$$$$)
{
	my ($proj, $warfile, $webbase, $webdir, $webwar) = @_;

	my $tgt = $proj->addTarget(TARGET_INSTALL_WAR,
				   "Install web application as WAR file.",
				   TARGET_WAR);
	$tgt->addNode('delete', dir => $webdir, quiet => 'true');
	$tgt->addNode('mkdir', dir => $webbase);
	$tgt->addNode('copy', file => $warfile, tofile => $webwar);
}

sub create_javadoc($\@$%)
{
	my ($proj, $srcdir, $destdir, %args) = @_;

	my $tgt = $proj->addTarget(TARGET_DOC,
				   'Build document from Java source files.');
	$tgt->addNode('delete', dir => $destdir, quiet => 'on');

	my (%attr) = (destdir => $destdir, 'use' => 'on', version => 'on',
		      windowtitle => '${ant.project.name} API',
		      failonerror => 'on', encoding => $ENCODING);
	my $classpath = $args{classpath};
	$attr{classpathref} = $classpath if ($classpath);
	my $javadoc = $tgt->addNode('javadoc', %attr);

	my $pattern = $args{pattern};
	foreach my $src (@$srcdir) {
		my $fset = $javadoc->addNode('fileset', dir => $src);
		$fset->addNode('patternset', refid => $pattern);
	}
}

sub create_junit($$$%)
{
	my ($proj, $target, $testjar, %args) = @_;

	my $tgt = $proj->addTarget($target, 'Run JUnit tests.', TARGET_JAR);
	my $failure = 'pkg.test.failure';
	my $junit = $tgt->addNode('junit', printsummary => 'withOutAndErr',
				  showoutput => 'on',
				  failureproperty => $failure);
	my $clpath = $junit->addNode('classpath');
	$clpath->addNode('pathelement', path => $testjar);
	$junit->addNode('formatter', type => 'plain', usefile => 'off');

	my $nameref = AntProperty::dereference(PROP_TESTNAME);
	$junit->addNode('test', name => $nameref);

	my $jnidir = $args{jnidir};
	if (defined($jnidir) and @$jnidir) {
		my $jdir = join(':', @$jnidir);
		$junit->addNode('sysproperty', key => SYSPROP_CORE_LIBPATH,
				value => $jdir);
	}

	$tgt->addNode('fail', message => 'JUnit test failed.', if => $failure);
}

sub create_classpath($$%)
{
	my ($proj, $id, %args) = @_;

	my $classpath = $args{classpath};
	my $extlibs = $args{extlibs};
	my $build_cp = $args{build_classpath};
	return undef unless ($classpath or $extlibs or $build_cp);

	my $pjardir = $args{jardir};
	my $pclasspath = $proj->addNode('path', id => $id);
	my $fset = $pclasspath->addNode('fileset', dir => $pjardir);
	foreach my $lib (@$classpath, @$extlibs) {
		$fset->addNode('include', name => $lib);
	}

	foreach my $lib (@$build_cp) {
		$pclasspath->addNode('pathelement', path => $lib);
	}

	return $pclasspath;
}
EOF
