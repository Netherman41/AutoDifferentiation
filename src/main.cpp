#include "MultiVarDiff.h"
#include <iostream>

int main() {
	multiVarDiff::Variable x;
	multiVarDiff::Variable y;		// different from x
	multiVarDiff::Variable z = x;	// same as x

	auto expression = x * z + 4 * y * y / (x + 5);

	// take partial derivatives
	auto dExpr_dx = expression.dx(x);
	auto dExpr_dy = expression.dx(y);

	// evaluate at concrete values
	std::cout << "dExpr_dx(x=10, y=200): " << dExpr_dx(x = 10, y = 200) << std::endl;
	std::cout << "dExpr_dy(x=10, y=200): " << dExpr_dy(x = 10, y = 200) << std::endl;

	return 0;
}