/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "kernels/Logistic.h"

#include "kernels/Utils.h"

#include <tensorflow/lite/kernels/internal/reference/logistic.h>

namespace luci_interpreter
{
namespace kernels
{

Logistic::Logistic(const Tensor *input, Tensor *output) : Kernel({input}, {output}) {}

void Logistic::configure()
{
  LUCI_INTERPRETER_CHECK(input()->element_type() == output()->element_type());
  if (input()->element_type() == DataType::U8)
  {
    LUCI_INTERPRETER_CHECK(output()->scale() == 1. / 256);
    populateLookupTable();
  }
  // TODO: enable it only if kernel with dynamic shapes
  output()->resize(input()->shape());
}

void Logistic::execute() const
{
  switch (input()->element_type())
  {
    case DataType::FLOAT32:
      evalFloat();
      break;
    case DataType::U8:
      evalQuantized();
      break;
    default:
      throw std::runtime_error("Unsupported type.");
  }
}

void Logistic::evalFloat() const
{
  if (_is_inplace)
    output()->set_data_buffer(const_cast<uint8_t *>(input()->data<uint8_t>()));

  tflite::reference_ops::Logistic(getTensorShape(input()), getTensorData<float>(input()),
                                  getTensorShape(output()), getTensorData<float>(output()));
  if (_is_inplace)
  {
    auto input_tensor = const_cast<Tensor *>(input());
    input_tensor->set_data_buffer(nullptr);
  }
}

void Logistic::evalQuantized() const
{
  const int size = tflite::MatchingFlatSize(getTensorShape(input()), getTensorShape(output()));
  uint8_t *output_data = getTensorData<uint8_t>(output());
  const uint8_t *input_data = getTensorData<uint8_t>(input());
  for (int i = 0; i < size; ++i)
  {
    output_data[i] = getTableValue(input_data[i]);
  }
}

void Logistic::populateLookupTable()
{
  const auto input_scale = static_cast<double>(input()->scale());
  const auto input_zero_point = static_cast<int32_t>(input()->zero_point());
  const auto output_scale = static_cast<double>(output()->scale());
  const auto output_zero_point = static_cast<int32_t>(output()->zero_point());
  const float inverse_scale = 1 / output_scale;
  int32_t maxval = std::numeric_limits<uint8_t>::max();
  int32_t minval = std::numeric_limits<uint8_t>::min();
  for (int32_t val = minval; val <= maxval; ++val)
  {
    const float dequantized = input_scale * (val - input_zero_point);
    const float transformed = 1.0f / (1.0f + std::exp(-dequantized));
    const float rescaled = std::round(transformed * inverse_scale);
    const int32_t quantized = static_cast<int32_t>(rescaled + output_zero_point);
    setTableValue(static_cast<uint8_t>(std::max(std::min(maxval, quantized), minval)),
                  static_cast<uint8_t>(val));
  }
}

} // namespace kernels
} // namespace luci_interpreter
