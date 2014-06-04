# Author: Daniel Ortiz Mart\'inez
# *- bash -*

# Target function to be used with the downhill package.  The target
# function adjusted by this script is the KSMR translation quality
# measure for the assisted translations generated by a computer assisted
# translation system.  The translations are generated using the
# "cat_using_client" tool, which is provided by the "thot" package.
# The "cat_using_client" tool requires the previous execution of the
# server "thot_server".

########
calc_nnc_pen()
{
    we="$1"
    nnc="$2"
    pen_fact=$3
    echo "$we" | $AWK -v nnc="${nnc}" -v pen_fact=${pen_fact}\
                      'BEGIN{
                             result=0;
                             split(nnc,nnc_arr," ")
                            }
                            {   
                             for(i=1;i<=NF;++i)
                             {
                              if($i<0 && nnc_arr[i]==1) 
                               result+=$i*pen_fact*(-1)
                             }
                            }
                         END{
                             printf"%f",result
                            }'
}

########
wait_until_server_is_listening()
{
    log_file=$1
    end=0
    num_retries=0
    max_num_retries=3
    while [ $end -eq 0 ]; do
        # Ensure server is being executed
        line=`ps aux | grep "thot_server" | grep ${PORT}`

        if [ -z "${line}" ]; then
            num_retries=`expr ${num_retries} + 1`
            if [ ${num_retries} -eq ${max_num_retries} ]; then
                echo "Error: server has terminated unexpectedly before start listening to port ${PORT}" >&2
                return 1
            fi
        fi

        # Check if server is listening
        line=`netstat -an | grep "LISTEN" | grep ":${PORT} "`
        if [ ! -z "${line}" ]; then
            end=1
        fi
        sleep 5
    done

## Alternative implementation (unelegant and error prone)
#     while [ $end -eq 0 ]; do
#         if [ "`tail -1 ${log_file} | $AWK '{printf"%s\n",$1}'`" = "Listening" ]; then
#             end=1
#         fi
#         sleep 5
#     done
}

####################### main

if [ $# -lt 2 ]; then
    echo "Usage: downhill_trgfunc_cat <sdir> <w1> ... <wn>"
else

    # Check server availability
    if [ ! -x $SERVER ]; then
        echo "Error: server cannot be executed!" >&2
        exit 1
    fi

    # Initialize variables
    if [ "${SERVER}" = "" ]; then SERVER=${bindir}/thot_server; fi
    if [ "${SERVER_IP}" = "" ]; then SERVER_IP="127.0.0.1" ; fi
    if [ "${CFGFILE}" = "" ]; then CFGFILE="server.cfg" ; fi
    if [ "${PORT}" != "" ]; then PORT_OPT="-p ${PORT}" ; fi
    if [ "${UID}" != "" ]; then UID_OPT="-uid ${UID}" ; fi
    if [ "${TEST}" = "" ]; then TEST=${BASEDIR}/raw/DATA/Es-dev ; fi
    if [ "${REF}" = "" ]; then REF=${BASEDIR}/raw/DATA/En-dev ; fi
    if [ "${TR}" = "1" ]; then TR_OPT="-tr"; fi
    if [ "${OF}" = "1" ]; then OF="-of" ; fi
    if [ "${VERBOSE_SERVER}" = "1" ]; then VERB_SERVER_OPT="-v" ; fi
    if [ "${PRINT_MODELS_PREF}" != "" ]; then PM_OPT="-pm ${PRINT_MODELS_PREF}" ; fi
    if [ "${NNC_PEN_FACTOR}" = "" ]; then NNC_PEN_FACTOR=1000; fi

    # Check variables
    if [ ! -f ${SERVER} ]; then
        echo "ERROR: file ${SERVER} does not exist" >&2
        exit 1
    fi

    if [ ! -f ${CFGFILE} ]; then
        echo "ERROR: file ${CFGFILE} does not exist" >&2
        exit 1
    fi

    if [ ! -f ${TEST} ]; then
        echo "ERROR: file ${TEST} does not exist" >&2
        exit 1
    fi

    if [ ! -f ${REF} ]; then
        echo "ERROR: file ${REF} does not exist" >&2
        exit 1
    fi

    # Read parameters
    SDIR=$1
    shift
    NUMW=$#
    weights=""
    while [ $# -gt 0 ]; do
        # Build weight vector
        weights="${weights} $1"
        shift
    done

    # Obtain non-negativity constraints penalty (non-negativity
    # constraints can be activated for each individual weight by means
    # of the environment variable NON_NEG_CONST, which contains a bit
    # vector; a value of 1 for i'th vector means that the i'th weight
    # must be positive)
    nnc_pen=0
    if [ ! "${NON_NEG_CONST}" = "" ]; then
        nnc_pen=`calc_nnc_pen "${weights}" "${NON_NEG_CONST}" ${NNC_PEN_FACTOR}`
    fi
 
    # # Generate cfg file for server
    # generate_cfg_file > ${SDIR}/server.cfg

    # Launch server
    $SERVER -c ${CFGFILE} ${PORT_OPT} ${VERB_SERVER_OPT} > ${SDIR}/server.log 2>&1 &
    server_pid=$!
    wait_until_server_is_listening ${SDIR}/server.log || error="yes"

    # Treat errors while launching server
    if [ "$error" = "yes" ]; then
        echo "Error while launching server, for additional information see file ${SDIR}/server.log" >&2
        exit 1
    fi

    # Kill server on exit
    trap "if [ ! -z \"\${server_pid}\" ]; then $bindir/thot_client -i ${SERVER_IP} ${PORT_OPT} -e; wait \${server_pid}; fi;" 0

    # Kill server if the script is aborted by means of Ctrl-C or SIGTERM
    trap "if [ ! -z \"\${server_pid}\" ]; then $bindir/thot_client -i ${SERVER_IP} ${PORT_OPT} -e; wait \${server_pid}; fi; exit 1" 2 15

    # Evaluate target function
    ${bindir}/thot_cat_using_client -i ${SERVER_IP} ${PORT_OPT} -t ${TEST} -r ${REF} ${TR_OPT} ${PM_OPT} ${OF} \
        > ${SDIR}/target_func.cat_iters 2> ${SDIR}/target_func.log || error="yes"

    # Treat errors while evaluating target function
    if [ "$error" = "yes" ]; then
        echo "Error while evaluating target function, for additional information see file ${SDIR}/target_func.log" >&2
        exit 1
    fi

    # Terminate server
    ${bindir}/thot_client -i ${SERVER_IP} ${PORT_OPT} ${UID_OPT} -e
    wait ${server_pid}
    server_pid=""

    # Obtain KSMR confidence intervals
    SEED=31415
    S_CI=`wc -l ${TEST} | $AWK '{printf"%d",$1}'`
    N_CI=1000
    ${bindir}/thot_conf_interv_cat $SEED ${SDIR}/target_func.cat_iters ${S_CI} ${N_CI} > ${SDIR}/target_func.conf_int

    # Calculate the KSMR measure
    grep "^KSMR" ${SDIR}/target_func.cat_iters | ${AWK} '{printf"KSMR= %s\n",$3}' >> ${SDIR}/target_func.ksmr

    # Obtain KSMR
    KSMR=`tail -1 ${SDIR}/target_func.ksmr | ${AWK} '{printf"%s",$2}'`
    
    # Print target function value
    echo "${KSMR} ${nnc_pen}" | $AWK '{printf"%f\n",$1+$2}'
fi
