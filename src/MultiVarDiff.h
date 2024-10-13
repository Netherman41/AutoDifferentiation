#pragma once
#include <type_traits>

// implements differentiation using multiple variables
namespace multiVarDiff {

	enum class ExprType : std::uint8_t {
		Constant,
		Variable,
		Sum,
		Difference,
		Product,
		Quotient
	};

	template <ExprType exprType, typename... Ts> struct Expression;

	// Evaluation variable - keeps track of which variable we're differentiating wrt
	struct EvalVariable {
		const Expression<ExprType::Variable>* initAddress;
		float value;
	};

	// Constant
	template <>
	struct Expression<ExprType::Constant> {
		template <typename T> constexpr Expression<ExprType::Constant>(T x) : value(static_cast<float>(x)) {}
		constexpr Expression<ExprType::Constant>() = default;
		constexpr float operator()(EvalVariable args...) const { return value; }
		constexpr auto dx(const Expression<ExprType::Variable>& args...) const { return Expression<ExprType::Constant>{}; }
		float value;
	};

	using Constant = Expression<ExprType::Constant>;

	// Variable
	template <>
	struct Expression<ExprType::Variable> {
		constexpr float operator()(const EvalVariable& x, std::convertible_to<const EvalVariable&> auto... args) const {
			return x.initAddress == initAddress ? x.value : operator()(args...);
		}
		
		constexpr float operator()(const EvalVariable& x) const { return x.initAddress == initAddress ? x.value : 0; }
		
		constexpr auto dx(const Expression<ExprType::Variable>& var) const { return Expression<ExprType::Constant>{ var.initAddress == initAddress ? 1 : 0 }; }

		constexpr EvalVariable operator=(float value) const { return EvalVariable{initAddress, value}; }
		Expression<ExprType::Variable>* initAddress = this;
	};

	using Variable = Expression<ExprType::Variable>;

	// Sum
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	struct Expression<ExprType::Sum, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>> {

		constexpr float operator()(std::convertible_to<const EvalVariable&> auto... args) const { return lhs(args...) + rhs(args...); }
		constexpr auto dx(const Expression<ExprType::Variable>& var) const { return lhs.dx(var) + rhs.dx(var); }

		Expression<exprType1, Ts1...> lhs;
		Expression<exprType2, Ts2...> rhs;
	};

	// sum of expressions
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	constexpr auto operator+(const Expression<exprType1, Ts1...>& lhs,
							 const Expression<exprType2, Ts2...>& rhs) {
		using TypeLHS = Expression<exprType1, Ts1...>;
		using TypeRHS = Expression<exprType2, Ts2...>;

		if constexpr (exprType1 == ExprType::Constant && exprType2 == ExprType::Constant) {
			return Constant{lhs.value + rhs.value};
		}
		else {
			return Expression<ExprType::Sum, TypeLHS, TypeRHS>{lhs, rhs};
		}
	}

	// Difference
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	struct Expression<ExprType::Difference, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>> {

		constexpr float operator()(std::convertible_to<const EvalVariable&> auto... args) const { return lhs(args...) - rhs(args...); }
		constexpr auto dx(const Expression<ExprType::Variable>& var) const { return lhs.dx(var) - rhs.dx(var); }

		Expression<exprType1, Ts1...> lhs;
		Expression<exprType2, Ts2...> rhs;
	};

	// difference of expressions
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	auto operator-(const Expression<exprType1, Ts1...>& lhs,
				   const Expression<exprType2, Ts2...>& rhs) {
		using TypeLHS = Expression<exprType1, Ts1...>;
		using TypeRHS = Expression<exprType2, Ts2...>;

		if constexpr (exprType1 == ExprType::Constant && exprType2 == ExprType::Constant) {
			return Expression<ExprType::Constant>{lhs.value - rhs.value};
		}
		else {
			return Expression<ExprType::Difference, TypeLHS, TypeRHS>{lhs, rhs};
		}
	}

	// Product
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	struct Expression<ExprType::Product, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>> {

		constexpr float operator()(std::convertible_to<const EvalVariable&> auto... args) const { return lhs(args...) * rhs(args...); }
		constexpr auto dx(const Expression<ExprType::Variable>& var) const { return lhs.dx(var) * rhs + lhs * rhs.dx(var); }

		Expression<exprType1, Ts1...> lhs;
		Expression<exprType2, Ts2...> rhs;
	};

	// product of expressions
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	auto operator*(const Expression<exprType1, Ts1...>& lhs,
				   const Expression<exprType2, Ts2...>& rhs) {
		using TypeLHS = Expression<exprType1, Ts1...>;
		using TypeRHS = Expression<exprType2, Ts2...>;

		if constexpr (exprType1 == ExprType::Constant && exprType2 == ExprType::Constant) {
			return Expression<ExprType::Constant>{lhs.value * rhs.value};
		}
		else {
			return Expression<ExprType::Product, TypeLHS, TypeRHS>{lhs, rhs};
		}
	}

	// Quotient
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	struct Expression<ExprType::Quotient, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>> {

		constexpr float operator()(std::convertible_to<const EvalVariable&> auto... args) const { return lhs(args...) / rhs(args...); }
		constexpr auto dx(const Expression<ExprType::Variable>& var) const { return (lhs.dx(var) * rhs - lhs * rhs.dx(var)) / (rhs * rhs); }

		Expression<exprType1, Ts1...> lhs;
		Expression<exprType2, Ts2...> rhs;
	};

	// quotient of expressions
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	auto operator/(const Expression<exprType1, Ts1...>& lhs,
				   const Expression<exprType2, Ts2...>& rhs) {
		using TypeLHS = Expression<exprType1, Ts1...>;
		using TypeRHS = Expression<exprType2, Ts2...>;

		if constexpr (exprType1 == ExprType::Constant && exprType2 == ExprType::Constant) {
			return Expression<ExprType::Constant>{lhs.value / rhs.value};
		}
		else {
			return Expression<ExprType::Quotient, TypeLHS, TypeRHS>{lhs, rhs};
		}
	}

	// global operators with floats
	template <ExprType exprType, typename... Ts>
	constexpr auto operator+(const Expression<exprType, Ts...>& lhs, float rhs) { return Expression<ExprType::Sum, Expression<exprType, Ts...>, Constant>{lhs, rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator+(float lhs, const Expression<exprType, Ts...>& rhs) { return Expression<ExprType::Sum, Constant, Expression<exprType, Ts...>>{lhs, rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator-(const Expression<exprType, Ts...>& lhs, float rhs) { return Expression<ExprType::Difference, Expression<exprType, Ts...>, Constant>{lhs, rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator-(float lhs, const Expression<exprType, Ts...>& rhs) { return Expression<ExprType::Difference, Constant, Expression<exprType, Ts...>>{lhs, rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator*(const Expression<exprType, Ts...>& lhs, float rhs) { return Expression<ExprType::Product, Expression<exprType, Ts...>, Constant>{lhs, rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator*(float lhs, const Expression<exprType, Ts...>& rhs) { return Expression<ExprType::Product, Constant, Expression<exprType, Ts...>>{lhs, rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator/(const Expression<exprType, Ts...>& lhs, float rhs) { return Expression<ExprType::Quotient, Expression<exprType, Ts...>, Constant>{lhs, rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator/(float lhs, const Expression<exprType, Ts...>& rhs) { return Expression<ExprType::Quotient, Constant, Expression<exprType, Ts...>>{lhs, rhs}; }



}



