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

#ifndef HLASMPLUGIN_PARSERLIBRARY_DEBUGGING_MACRO_PARAM_VARIABLE_H
#define HLASMPLUGIN_PARSERLIBRARY_DEBUGGING_MACRO_PARAM_VARIABLE_H

#include "variable.h"
#include "../context/variables/macro_param.h"

namespace hlasm_plugin::parser_library::debugging
{

class macro_param_variable : public variable
{
public:
	macro_param_variable(const context::macro_param_base& param, std::vector<size_t> index);
	virtual set_type type() const override;

	virtual bool is_scalar() const override;

	virtual std::vector<variable_ptr> values() const override;
	virtual size_t size() const override;
protected:
	virtual const std::string& get_string_value() const override;
	virtual const std::string& get_string_name() const override;
private:
	const context::macro_param_base& macro_param_;
	std::vector<size_t> index_;
};


}


#endif // !HLASMPLUGIN_PARSERLIBRARY_DEBUGGING_MACRO_PARAM_VARIABLE_H
