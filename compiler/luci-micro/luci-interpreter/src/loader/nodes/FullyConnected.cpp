/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd. All Rights Reserved
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

#include "Builders.h"

#include "kernels/FullyConnected.h"

namespace luci_interpreter
{

std::unique_ptr<Kernel> build_kernel_CircleFullyConnected(std::vector<const Tensor *> &&inputs,
                                                          std::vector<Tensor *> &&outputs,
                                                          const uint32_t op_index,
                                                          KernelBuilder &builder)
{
  assert(inputs.size() == 3);

  const Tensor *input = inputs.at(0);
  const Tensor *weights = inputs.at(1);
  const Tensor *bias = inputs.at(2);
  Tensor *output = outputs.at(0);

  circle::OperatorT oper_t;
  builder.get_circle_reader()->operators()[op_index]->UnPackTo(&oper_t);
  const auto *options = oper_t.builtin_options.AsFullyConnectedOptions();

  FullyConnectedParams params{};
  params.activation = luci_actfunc(options->fused_activation_function);
  params.keep_num_dims = options->keep_num_dims;

  return std::make_unique<kernels::FullyConnected>(input, weights, bias, output, params);
}

} // namespace luci_interpreter
