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

#ifndef SEMANTICS_OPERAND_H
#define SEMANTICS_OPERAND_H

#include <vector>

#include "concatenation.h"
#include "../expressions/mach_expression.h"
#include "../expressions/data_definition.h"
#include "../checking/instr_operand.h"
#include "../checking/data_definition/data_definition_operand.h"

namespace hlasm_plugin {
namespace parser_library {
namespace semantics {

enum class operand_type
{
	MACH, MAC, ASM, CA, DAT, MODEL, EMPTY
};

struct model_operand;
struct ca_operand;
struct macro_operand;
struct machine_operand;
struct assembler_operand;
struct data_def_operand;

struct operand;
using operand_ptr = std::unique_ptr<operand>;
using operand_list = std::vector<operand_ptr>;
using remark_list = std::vector<range>;

struct op_rem
{
	std::vector<operand_ptr> operands;
	std::vector<range> remarks;
};

struct seq_sym
{
	context::id_index name;
	range symbol_range;
};

//struct representing operand of instruction
struct operand
{
	operand(const operand_type type, const range operand_range);

	model_operand* access_model();
	ca_operand* access_ca();
	macro_operand* access_mac();
	data_def_operand* access_data_def();
	machine_operand* access_mach();
	assembler_operand* access_asm();

	const operand_type type;
	const range operand_range;

	virtual ~operand() = default;
};



struct empty_operand final : public operand
{
	empty_operand(const range operand_range);
};



//operand that contains variable symbol thus is 'model operand'
struct model_operand final : public operand
{
	model_operand(concat_chain chain, const range operand_range);

	concat_chain chain;
};



struct evaluable_operand : public operand, public diagnosable_op_impl
{
	evaluable_operand(const operand_type type, const range operand_range);

	virtual bool has_dependencies(expressions::mach_evaluate_info info) const = 0;

	virtual bool has_error(expressions::mach_evaluate_info info) const = 0;

	virtual std::vector<const context::resolvable*> get_resolvables() const = 0;

	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info) const = 0;
};



struct simple_expr_operand : public virtual evaluable_operand
{
	simple_expr_operand(expressions::mach_expr_ptr expression);

	virtual bool has_dependencies(expressions::mach_evaluate_info info) const override;

	virtual bool has_error(expressions::mach_evaluate_info info) const override;

	virtual std::vector<const context::resolvable*> get_resolvables() const override;

	expressions::mach_expr_ptr expression;
};


enum class mach_kind {EXPR,ADDR};
struct expr_machine_operand;
struct address_machine_operand;

struct machine_operand : public virtual evaluable_operand
{
	machine_operand(const mach_kind kind);

	expr_machine_operand* access_expr();
	address_machine_operand* access_address();
	
	using evaluable_operand::get_operand_value;
	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info, checking::machine_operand_type type_hint) const = 0;

	const mach_kind kind;
};



struct expr_machine_operand final : public machine_operand, public simple_expr_operand
{
	expr_machine_operand(expressions::mach_expr_ptr expression, const range operand_range);

	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info) const override;
	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info, checking::machine_operand_type type_hint) const override;

	virtual void collect_diags() const override;
};



struct address_machine_operand final : public machine_operand
{
	address_machine_operand(
		expressions::mach_expr_ptr displacement, 
		expressions::mach_expr_ptr first_par, 
		expressions::mach_expr_ptr second_par, 
		const range operand_range,
		checking::operand_state state);
	
	expressions::mach_expr_ptr displacement;
	expressions::mach_expr_ptr first_par;
	expressions::mach_expr_ptr second_par;
	checking::operand_state state;

	virtual bool has_dependencies(expressions::mach_evaluate_info info) const override;

	virtual bool has_error(expressions::mach_evaluate_info info) const override;

	virtual std::vector<const context::resolvable*> get_resolvables() const override;

	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info) const override;
	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info, checking::machine_operand_type type_hint) const override;

	virtual void collect_diags() const override;
};


enum class asm_kind {EXPR, BASE_END, COMPLEX, STRING};
struct expr_assembler_operand;
struct using_instr_assembler_operand;
struct complex_assembler_operand;
struct string_assembler_operand;

struct assembler_operand : public virtual evaluable_operand
{
	assembler_operand(const asm_kind kind);

	expr_assembler_operand* access_expr();
	using_instr_assembler_operand* access_base_end();
	complex_assembler_operand* access_complex();
	string_assembler_operand* access_string();

	const asm_kind kind;
};


struct expr_assembler_operand final : public assembler_operand, public simple_expr_operand
{
private:
	std::string value_;
public:
	expr_assembler_operand(expressions::mach_expr_ptr expression, std::string string_value, const range operand_range);

	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info) const override;

	virtual void collect_diags() const override;
};



struct using_instr_assembler_operand final : public assembler_operand
{
	using_instr_assembler_operand(expressions::mach_expr_ptr base, expressions::mach_expr_ptr end, const range operand_range);

	virtual bool has_dependencies(expressions::mach_evaluate_info info) const override;

	virtual bool has_error(expressions::mach_evaluate_info info) const override;

	virtual std::vector<const context::resolvable*> get_resolvables() const override;

	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info) const override;

	expressions::mach_expr_ptr base;
	expressions::mach_expr_ptr end;

	virtual void collect_diags() const override;
};



struct complex_assembler_operand final : public assembler_operand
{
	struct component_value_t 
	{
		component_value_t() : op_range(range()) {}
		component_value_t(range op_range) : op_range(op_range) {}

		virtual std::unique_ptr<checking::asm_operand> create_operand() const= 0;
		virtual ~component_value_t() = default;

		range op_range;
	};

	struct int_value_t final : public component_value_t
	{
		int_value_t(int value, range range) : component_value_t(range), value(value) {}
		virtual std::unique_ptr<checking::asm_operand> create_operand() const override { return std::make_unique<checking::one_operand>(value, op_range); }
		int value;
	};
	struct string_value_t final : public component_value_t
	{
		//string_value_t(std::string value) : value(std::move(value)) {}
		string_value_t(std::string value, range range) : component_value_t(range), value(std::move(value)) {}
		virtual std::unique_ptr<checking::asm_operand> create_operand() const override { return std::make_unique<checking::one_operand>(value, op_range); }
		std::string value;
	};
	struct composite_value_t final : public component_value_t
	{
		composite_value_t(std::string identifier, std::vector<std::unique_ptr<component_value_t>> values, range range)
			: component_value_t(range), identifier(std::move(identifier)), values(std::move(values)) {}
		virtual std::unique_ptr<checking::asm_operand> create_operand() const override
		{ 
			std::vector<std::unique_ptr<checking::asm_operand>> ret;
			for (auto& val : values)
				ret.push_back(val->create_operand());
			return std::make_unique<checking::complex_operand>(identifier,std::move(ret));
		}

		std::string identifier;
		std::vector<std::unique_ptr<component_value_t>> values;
	};

	complex_assembler_operand(std::string identifier, std::vector<std::unique_ptr<component_value_t>> values, const range operand_range);

	virtual bool has_dependencies(expressions::mach_evaluate_info info) const override;

	virtual bool has_error(expressions::mach_evaluate_info info) const override;

	virtual std::vector<const context::resolvable*> get_resolvables() const override;

	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info) const override;

	composite_value_t value;

	virtual void collect_diags() const override;
};



struct string_assembler_operand : public assembler_operand
{

	string_assembler_operand(std::string value, const range operand_range);

	virtual bool has_dependencies(expressions::mach_evaluate_info info) const override;

	virtual bool has_error(expressions::mach_evaluate_info info) const override;

	virtual std::vector<const context::resolvable*> get_resolvables() const override;

	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info) const override;

	std::string value;

	virtual void collect_diags() const override;
};

//TODO operand_range*************
struct data_def_operand final : public evaluable_operand
{
	data_def_operand(expressions::data_definition data_def, const range operand_range);

	std::shared_ptr<expressions::data_definition> value;

	context::dependency_collector get_length_dependencies(expressions::mach_evaluate_info info) const;

	context::dependency_collector get_dependencies(expressions::mach_evaluate_info info) const;

	virtual bool has_dependencies(expressions::mach_evaluate_info info) const override;

	virtual bool has_error(expressions::mach_evaluate_info info) const override;

	virtual std::vector<const context::resolvable*> get_resolvables() const override;

	virtual std::unique_ptr<checking::operand> get_operand_value(expressions::mach_evaluate_info info) const override;

	virtual void collect_diags() const override;
};


enum class ca_kind { VAR, EXPR, SEQ, BRANCH };
struct var_ca_operand;
struct expr_ca_operand;
struct seq_ca_operand;
struct branch_ca_operand;

struct ca_operand : public operand
{
	ca_operand(const ca_kind kind, const range operand_range);

	var_ca_operand* access_var();
	const var_ca_operand* access_var() const;
	expr_ca_operand* access_expr();
	const expr_ca_operand* access_expr() const;
	seq_ca_operand* access_seq();
	const seq_ca_operand* access_seq() const;
	branch_ca_operand* access_branch();
	const branch_ca_operand* access_branch() const;

	const ca_kind kind;
};

struct var_ca_operand final : public ca_operand
{
	var_ca_operand(vs_ptr variable_symbol, const range operand_range);

	vs_ptr variable_symbol;
};

struct expr_ca_operand final : public ca_operand
{
	expr_ca_operand(antlr4::ParserRuleContext* expression, const range operand_range);

	antlr4::ParserRuleContext* expression;
};

struct seq_ca_operand final : public ca_operand
{
	seq_ca_operand(seq_sym sequence_symbol, const range operand_range);

	seq_sym sequence_symbol;
};

struct branch_ca_operand final : public ca_operand
{
	branch_ca_operand(seq_sym sequence_symbol, antlr4::ParserRuleContext* expression, const range operand_range);

	seq_sym sequence_symbol;
	antlr4::ParserRuleContext* expression;
};



struct macro_operand final : public operand
{
	macro_operand(concat_chain chain, const range operand_range);

	concat_chain chain;
};

struct macro_operand_string final : public operand
{
	macro_operand_string(std::string value, const range operand_range);

	std::string value;
};

}
}
}
#endif
