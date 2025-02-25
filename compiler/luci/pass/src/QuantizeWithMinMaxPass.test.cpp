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

#include "luci/Pass/QuantizeWithMinMaxPass.h"

#include <luci/IR/CircleNodes.h>

#include <gtest/gtest.h>

class SimpleConcatGraph
{
public:
  SimpleConcatGraph(loco::DataType quant_type)
  {
    concat_node = g.nodes()->create<luci::CircleConcatenation>(2);
    input_1 = g.nodes()->create<luci::CircleConst>();
    input_2 = g.nodes()->create<luci::CircleConst>();

    concat_node->dtype(quant_type);
    concat_node->fusedActivationFunction(luci::FusedActFunc::NONE);
    input_1->dtype(quant_type);
    input_2->dtype(quant_type);

    concat_node->values(0, input_1);
    concat_node->values(1, input_2);
  }

  ~SimpleConcatGraph()
  {
    concat_node->values(0, nullptr);
    concat_node->values(1, nullptr);
  }

public:
  loco::Graph g;
  luci::CircleConcatenation *concat_node = nullptr;
  luci::CircleConst *input_1 = nullptr;
  luci::CircleConst *input_2 = nullptr;
};

TEST(QuantizeWithMinMaxPassTest, name)
{
  luci::QuantizeWithMinMaxPass pass(loco::DataType::FLOAT32, loco::DataType::U8,
                                    luci::QuantizationGranularity::LayerWise);
  auto const name = pass.name();
  ASSERT_NE(nullptr, name);
}

// Test concat of integer tensors
// Integer tensors are not quantized
TEST(QuantizeWithMinMaxPassTest, int_concat)
{
  SimpleConcatGraph g(loco::DataType::S32);

  luci::QuantizeWithMinMaxPass qwmm(loco::DataType::FLOAT32, loco::DataType::U8,
                                    luci::QuantizationGranularity::LayerWise);

  qwmm.run(&g.g);

  EXPECT_EQ(nullptr, g.concat_node->quantparam());
  EXPECT_EQ(nullptr, g.input_1->quantparam());
  EXPECT_EQ(nullptr, g.input_2->quantparam());
}
