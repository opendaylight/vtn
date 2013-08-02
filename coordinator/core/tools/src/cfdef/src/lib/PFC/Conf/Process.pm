#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Sub process generator.
##

package PFC::Conf::Process;

use strict;
use FileHandle;
use POSIX qw(:DEFAULT :sys_wait_h :errno_h :signal_h);

=head1 NAME

PFC::Conf::Process - Sub process generator

=head1 SYNOPSIS

  #
  # Execute "/bin/ls -al" on a sub process.
  #
  my $proc = PFC::Conf::Process->new(NAME => "ls");
  my $func = sub {
      exec("/bin/ls", "-al");
  };
  $proc->start($func);
  $proc->wait(FATAL => 1);

  #
  # Execute "/bin/cat FILE | /bin/grep foo".
  #
  my $proc1 = PFC::Conf::Process->new(NAME => "cat", OUT => undef);
  my $func1 = sub {
      exec("/bin/cat", "FILE");
  };
  $proc1->start($func1);
  my $out1 = $proc1->getOutHandle();

  my $proc2 = PFC::Conf::Process->new(NAME => "grep", IN => $out1);
  my $func2 = sub {
      exec("/bin/grep", "foo");
  };
  $proc2->start($func2);

  $proc1->wait(FATAL => 1);
  my $status = $proc2->wait();

=head1 ABSTRACT

B<PFC::Conf::Process> creates a sub process, and executes the given
function on a sub process.

An instance of B<PFC::Conf::Process> represents a sub process, so it can
create only one sub process. After execution, an instance of
B<PFC::Conf::Process> must be discarded.

=head1 DESCRIPTION

This section describes about public interface for B<PFC::Conf::Process>.

=head2 METHODS

=over 4

=item I<new>(%args)

Constructor.

Arguments must be specified by a HASH object.
The following HASH keys are recognized:

=over 4

=item B<NAME>

Specifies a symbolic name of sub process.
This value is used to create an error message.

=item B<IN>

Indicates that the standard input of a sub process must be bound to a pipe.

If an instance of B<FileHandle> is associated with this key, it is bound
to the standard input of a sub process.

Otherwise, the value is treated as file name. The given file is redirected
to the standard input of a sub process.

=item B<OUT>

Indicates that the standard output of a sub process must be bound to a pipe.

If undefined value is associated with this key, the caller can receive output
of a sub process via B<FileHandle> instance returned by B<getOutHandle>
method.

Otherwise, the value is treated as file name. Output of a sub process is
redirected to the given file.

=back

=cut

sub new
{
	my $this = shift;
	my $class = ref($this) || $this;
	my (%args) = @_;

	return bless(\%args, $class);
}

=item I<start>($func)

Create a sub process, and execute the given function on it.

I<$func> must be a reference to function to be executed.
It must terminate a sub process by calling B<exit> on successful completion.
Otherwise a sub process exits with status 255.

An exception is thrown on error.

=cut

sub start
{
	my $me = shift;
	my ($func) = @_;

	my $in = $me->setupInput();
	my ($orh, $owh) = $me->setupOutput();

	# Create a sub process.
	my $pid = fork();
	$me->fatal("fork() failed: $!") unless (defined($pid));

	if ($pid) {
		# Preserve handle associated with stdout.
		$me->{OUTHANDLE} = $orh if ($orh);
		$me->{PID} = $pid;

		return;
	}

	# Set up stdin.
	if ($in) {
		POSIX::dup2($in->fileno(), STDIN->fileno()) or
			$me->fatal("dup2() failed: $!");
	}

	# Set up stdout.
	undef $orh if ($orh);
	POSIX::dup2($owh->fileno(), STDOUT->fileno()) or
		$me->fatal("dup2() failed: $!");

	# Call the given function.
	&$func();

	$me->fatal("Function returned unexpectedly.");
	exit 255;
}

=item I<wait>(%args)

Wait for a sub process to quit.

Arguments must be specified by a HASH object.
The following HASH keys are recognized:

=over 4

=item B<FATAL>

Check exit status of a sub process.

If true value is associated with this key, I<wait> throws an exception
unless a sub process exits with status 0.

=back

This function returns an exit status of a sub process.

=cut

sub wait
{
	my $me = shift;
	my (%args) = @_;

	my $pid = $me->{PID};
	delete($me->{PID});
	$me->fatal("Sub process is not yet created.") unless ($pid);

	while (1) {
		my $cpid = waitpid($pid, 0);
		last unless ($cpid == -1);
		$me->fatal("waitpid($pid) failed: $!") unless ($! == EINTR);
	}

	my $status = $?;
	if (WIFEXITED($status)) {
		my $st = WEXITSTATUS($status);
		return $st if ($st == 0 or !$args{FATAL});

		my $name = $me->{NAME};
		my $proc = ($name) ? "($name)" : "";
		die "Sub process$proc exited with status $st.\n";
	}

	if (WIFSIGNALED($status)) {
		my $sig = WTERMSIG($status);

		my $name = $me->{NAME};
		my $proc = ($name) ? "($name)" : "";
		die "Sub process$proc was killed by signal $sig.\n";
	}

	my $name = $me->{NAME};
	my $proc = ($name) ? "($name)" : "";
	my $st = sprintf("0x%x", $status);
	die "Sub process$proc quit abnormally: status=$st\n";
}

=item I<getOutHandle>()

Return a file handle associated with the standard output of a sub process.

This method returns undefined value if "OUT => undef" is not specified to
the constructor.

=cut

sub getOutHandle
{
	my $me = shift;

	return $me->{OUTHANDLE}
}

=back

=cut

##
## Below are private methods.
##

#
# Destructor.
#
sub DESTROY
{
	my $me = shift;

	my $pid = $me->{PID};
	return unless ($pid);

	# Determine whether a sub process is still running.
	my $cpid = waitpid($pid, WNOHANG);
	return unless ($cpid == 0);

	# Kill this process by SIGKILL.
	kill(SIGKILL, $pid);

	do {
		$cpid = waitpid($pid, 0);
	} while ($cpid == -1 and $! == EINTR);
}

sub setupInput
{
	my $me = shift;

	my $in = $me->{IN};
	return undef unless ($in);

	my $handle;
	eval {
		$handle = 1 if ($in->can('fileno'));
	};
	return $in if ($handle);

	# Redirect the given file to stdin.
	my $fh = FileHandle->new($in);
	$me->fatal("open($in) failed: $!") unless ($fh);

	return $fh;
}

sub setupOutput
{
	my $me = shift;

	return undef unless (exists($me->{OUT}));

	my ($rh, $wh);
	my $out = $me->{OUT};
	if ($out) {
		# Redirect output to the given file.
		$wh = FileHandle->new($out, O_WRONLY | O_CREAT | O_TRUNC,
				      0644);
		$me->fatal("open($out) failed: $!") unless ($wh);
	}
	else {
		# Redirect output to a pipe.
		($rh, $wh) = FileHandle::pipe();
		$me->fatal("pipe() failed: $!") unless ($rh);
	}

	return ($rh, $wh);
}

sub fatal
{
	my $me = shift;

	my $name = $me->{NAME};
	my $proc = ($name) ? "$name: " : "";
	my $msg = join('', $proc, @_);

	die "$msg\n";
}

=head1 SEE ALSO

B<FileHandle>(3)

=head1 AUTHOR

NEC Corporation

=cut

1;
