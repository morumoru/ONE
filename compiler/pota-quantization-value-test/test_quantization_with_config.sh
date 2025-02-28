#!/bin/bash

# This script tests quantize_with_minmax option of circle-quantizer with config file
#
# HOW TO USE
#
# ./test_quantization_with_config.sh <path/to/test.config> <path/to/work_dir> <TEST 1> <TEST 2> ...
# test.config : set ${RECORD_MINMAX_PATH} and ${CIRCLE_QUANTIZER_PATH}
# work_dir : build directory of quantization-value-test (ex: build/compiler/quantization-value-test)

SOURCE_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMPARE_SCRIPT_PATH="${SOURCE_PATH}/compare_tensors.py"
CONFIG_PATH="$1"; shift
BIN_PATH=$(dirname "${CONFIG_PATH}")
TEST_INPUT_PATH="${SOURCE_PATH}/test_inputs"
GEN_SCRIPT_PATH="${BIN_PATH}/gen_h5_explicit_inputs.py"
WORKDIR="$1"; shift

source "${CONFIG_PATH}"

echo "-- Found CIRCLE_QUANTIZER: ${CIRCLE_QUANTIZER_PATH}"
echo "-- Found CIRCLE_TENSORDUMP: ${CIRCLE_TENSORDUMP_PATH}"
echo "-- Found workdir: ${WORKDIR}"

TESTED=()
PASSED=()
FAILED=()

pushd "${WORKDIR}"
while [ "$1" != "" ]; do  
  MODELNAME=$1; shift
  GRANULARITY=$1; shift
  DTYPE=$1; shift
  TESTCASE="${MODELNAME}.${GRANULARITY}.${DTYPE}"

  TESTED+=("${TESTCASE}")

  TESTCASE_FILE="${WORKDIR}/${TESTCASE}"
  TEST_RESULT_FILE="${BIN_PATH}/${TESTCASE}"

  PASSED_TAG="${TEST_RESULT_FILE}.quantization.mixed.passed"
  rm -f "${PASSED_TAG}"

  cat > "${TEST_RESULT_FILE}_quantization_with_config.log" <(
    exec 2>&1
    set -ex

    # Generate h5 input data
    source "${VIRTUALENV}/bin/activate"
    "${VIRTUALENV}/bin/python" "${GEN_SCRIPT_PATH}" \
      --model "${WORKDIR}/${MODELNAME}.circle" \
      --input "${TEST_INPUT_PATH}/${MODELNAME}_config/${GRANULARITY}/${DTYPE}" \
      --output "${TESTCASE_FILE}.mixed.input.h5"

    if [[ $? -ne 0 ]]; then
      echo "FAILED TO GENERATE INPUT"
      continue
    fi

    # Run record-minmax
    # NOTE There is no '_with_config' test for record-minmax, because it does not
    # use quantization config file.
    "${RECORD_MINMAX_PATH}" \
      --input_model "${TEST_RESULT_FILE}.fake_quantized.mixed.circle" \
      --input_data "${TESTCASE_FILE}.mixed.input.h5" \
      --output_model "${TEST_RESULT_FILE}.minmax_recorded.mixed.circle" 

    # Run circle-quantizer with --quantize_with_minmax
    "${CIRCLE_QUANTIZER_PATH}" \
      --quantize_with_minmax float32 "${DTYPE}" "${GRANULARITY}" \
      --config "${SOURCE_PATH}/config_files/${MODELNAME}/${GRANULARITY}/${DTYPE}/qconf.json" \
      "${TEST_RESULT_FILE}.minmax_recorded.mixed.circle" \
      "${TEST_RESULT_FILE}.quantized.mixed.circle" 

    # Dump scale, zp, weights values (circle-tensordump)
    "${CIRCLE_TENSORDUMP_PATH}" \
      "${TEST_RESULT_FILE}.quantized.mixed.circle" \
      --tensors_to_hdf5 "${TEST_RESULT_FILE}.quantized.mixed.circle.h5"

    # Compare result
    "${VIRTUALENV}/bin/python" "${COMPARE_SCRIPT_PATH}" \
      --input_h5 "${TEST_RESULT_FILE}.quantized.mixed.circle.h5" \
      --expect_dir "${SOURCE_PATH}/expected_outputs/${MODELNAME}_config/${GRANULARITY}/${DTYPE}/quantization" \
      --mode quantization

    if [[ $? -eq 0 ]]; then
      touch "${PASSED_TAG}"
    fi
  )

  if [[ -f "${PASSED_TAG}" ]]; then
    PASSED+=("$TESTCASE")
  else
    FAILED+=("$TESTCASE")
  fi
done
popd

if [[ ${#TESTED[@]} -ne ${#PASSED[@]} ]]; then
  echo "FAILED"
  for TEST in "${FAILED[@]}"
  do
    echo "- ${TEST}"
  done
  exit 255
fi

echo "PASSED"
exit 0
