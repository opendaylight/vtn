#!/bin/sh

#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Start OpenDaylight controller with VTN Manager.
##

PATH=/bin:/sbin:/usr/bin:/usr/sbin
export PATH

PROGNAME=`basename $0`
RUNDIR=`dirname $0`
RUN_CONTROLLER=$RUNDIR/run_controller.sh

CONFDIR=configuration
CONF_INITIAL=$CONFDIR/initial
INIT_AVAIL=initial.available

help()
{
    cat <<EOF
Usage: $0 [options]

Common Options:
EOF
    $RUN_CONTROLLER -help | sed -e '/^For other information type/i\
Additional Options:\
\
     of10             [-of10]\
     of13             [-of13]\
'
    exit 1
}

ARGS=
while [ $# -ne 0 ]; do
    case "$1" in
        -of10)
            # Use legacy AD-SAL openflow plugin.
            OF13=0
            ;;

        -of13)
            # Use MD-SAL openflow plugin.
            OF13=1
            ;;

        -bundlefilter)
            # Specify filter to be passed to fileinstall.
            shift
            BUNDLEFILTER="|$1"
            ;;

        -help)
            help
            ;;

        *)
            # Escape double quotation marks.
            arg=`echo $1 | sed 's,",\\\",g'`
            ARGS="$ARGS \"$arg\""
            ;;
    esac
    shift
done

# Clean up symbolic links under configuration/initial directory.
find $CONF_INITIAL -type l -exec rm '{}' \;

# Set up openflow plugin.
OF_FILTER="org\.opendaylight\.(openflowplugin|openflowjava|controller\.sal-compatibility)"

if [ "$OF13" = 1 ]; then
    OF_FILTER="org\.opendaylight\.controller\.(thirdparty\.org\.openflow|protocol_plugins\.openflow)"

    # Instal configuration for openflow plugin.
    find $CONFDIR/$INIT_AVAIL -name '*openflowplugin*.xml' | \
        while read cf; do
            cfname=`basename $cf`
            ln -s ../$INIT_AVAIL/$cfname $CONF_INITIAL
        done
fi

# Build the bundle filter string.
FILTER_BEGIN='^(?!'
FILTER_END=').*'
FILTER="${FILTER_BEGIN}${OF_FILTER}${BUNDLEFILTER}${FILTER_END}"
FILTER_ARG="-Dfelix.fileinstall.filter=\"$FILTER\""

# Start the controller.
eval exec $RUN_CONTROLLER $FILTER_ARG $ARGS
