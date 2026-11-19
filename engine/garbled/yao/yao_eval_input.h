/*
 * garbled/yao/yao_eval_input.h
 *
 */

#pragma once

#include "garbled/yao/yao_evaluator.h"

class YaoEvalInput : public InputterBase
{
public:
	YaoEvaluator& evaluator;
	BitVector inputs;
	int i_bit;
	OctetStream os;

	YaoEvalInput() :
			evaluator(YaoEvaluator::s())
	{
		inputs.resize(0);
		i_bit = 0;
	}

	void exchange()
	{
		evaluator.ot_ext.extend_correlated(inputs.size(), inputs);
		evaluator.player.receive(os);
	}
};
