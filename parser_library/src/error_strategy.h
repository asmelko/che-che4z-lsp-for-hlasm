/*
 * Copyright (c) 2019 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Broadcom, Inc. - initial API and implementation
 */

#ifndef HLASMPLUGIN_PARSERLIBRARY_ERROR_STRATEGY_H
#define HLASMPLUGIN_PARSERLIBRARY_ERROR_STRATEGY_H

#include "DefaultErrorStrategy.h"
#include "shared/lexer.h"

namespace hlasm_plugin::parser_library
{

enum tokens { 
	#include "grammar/lex.tokens" 
};

class error_strategy : public antlr4::DefaultErrorStrategy
{
	virtual void reportError(antlr4::Parser *recognizer, const antlr4::RecognitionException &e) override
	{
		if (inErrorRecoveryMode(recognizer)) {
			return; // don't report spurious errors
		}

		//recovery strategy
		antlr4::misc::IntervalSet endTokens;

		endTokens.addItems(tokens::EOLLN);
		consumeUntil(recognizer, endTokens);

		beginErrorCondition(recognizer);
		if (antlrcpp::is<const antlr4::NoViableAltException *>(&e)) {
			reportNoViableAlternative(recognizer, static_cast<const antlr4::NoViableAltException &>(e));
		}
		else if (antlrcpp::is<const antlr4::InputMismatchException *>(&e)) {
			reportInputMismatch(recognizer, static_cast<const antlr4::InputMismatchException &>(e));
		}
		else if (antlrcpp::is<const antlr4::FailedPredicateException *>(&e)) {
			reportFailedPredicate(recognizer, static_cast<const antlr4::FailedPredicateException &>(e));
		}
		else if (antlrcpp::is<const antlr4::RecognitionException *>(&e)) {
			recognizer->notifyErrorListeners(e.getOffendingToken(), e.what(), std::current_exception());
		}
	}

	virtual antlr4::Token* getMissingSymbol(antlr4::Parser *recognizer) override
	{
		using namespace antlr4;

		token * currentSymbol = dynamic_cast<token *>(recognizer->getCurrentToken());
		assert(currentSymbol);
		
		misc::IntervalSet expecting = getExpectedTokens(recognizer);
		size_t expectedTokenType = expecting.getMinElement(); // get any element
		std::string tokenText;
		if (expectedTokenType == Token::EOF) {
			tokenText = "Unexpected end of file";
		}
		else {
			tokenText = "<missing " + recognizer->getVocabulary().getDisplayName(expectedTokenType) + ">";
		}
		token *current = currentSymbol;
		token *lookback = dynamic_cast<token *>(recognizer->getTokenStream()->LT(-1));
		if (current->getType() == Token::EOF && lookback != nullptr) {
			current = lookback;
		}

		lexer * lex = dynamic_cast<lexer *>(recognizer->getTokenStream()->getTokenSource());

		_errorSymbols.push_back(lex->get_token_factory()->create(current->getTokenSource(), current->getInputStream(),
			expectedTokenType, Token::DEFAULT_CHANNEL, INVALID_INDEX, INVALID_INDEX,
			current->getLine(), current->getCharPositionInLine(), (size_t) -1, current->get_char_position_in_line_16(), current->get_end_of_token_in_line_utf16()));

		return _errorSymbols.back().get();
	}

	virtual antlr4::Token* singleTokenDeletion(antlr4::Parser* ) override
	{
		return nullptr;
	}

	std::vector<std::unique_ptr<antlr4::Token>> _errorSymbols;
};

}


#endif