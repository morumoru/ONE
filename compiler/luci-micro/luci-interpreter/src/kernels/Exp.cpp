/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
 * Copyright 2018 The TensorFlow Authors. All Rights Reserved.
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

#include "kernels/Exp.h"

#include "kernels/Utils.h"

#include <tensorflow/lite/kernels/internal/reference/exp.h>

namespace luci_interpreter
{
namespace kernels
{

Exp::Exp(const Tensor *input, Tensor *output) : Kernel({input}, {output}) {}

void Exp::configure()
{
  LUCI_INTERPRETER_CHECK(input()->element_type() == output()->element_type());
  // TODO: enable it only if kernel with dynamic shapes
  output()->resize(input()->shape());
}

void Exp::execute() const
{
  switch (input()->element_type())
  {
    case DataType::FLOAT32:
      evalFloat();
      break;
    default:
      throw std::runtime_error("Unsupported type.");
  }
}

void Exp::evalFloat() const
{
  const int size = tflite::MatchingFlatSize(getTensorShape(input()), getTensorShape(output()));
  tflite::reference_ops::Exp(getTensorData<float>(input()), size, getTensorData<float>(output()));
}

} // namespace kernels
} // namespace luci_interpreter
