#pragma once
#include <type_traits>

// implements differentiation by single variable
namespace singleVarDiff {

	enum class ExprType : std::uint8_t {
		Constant,
		Variable,
		Sum,
		Difference,
		Product,
		Quotient
	};

	struct Zero {};
	struct One {};

	template <ExprType exprType, typename... Ts> struct Expression;

	// Constant
	template <>
	struct Expression<ExprType::Constant, Zero> {
		constexpr float operator()(float x) { return 0; }
		constexpr auto dx() { return Expression<ExprType::Constant, Zero>{}; }
		static constexpr float value = 0;
	};
	using ZeroExpr = Expression<ExprType::Constant, Zero>;

	template <>
	struct Expression<ExprType::Constant, One> {
		constexpr float operator()(float x) { return 1; }
		constexpr auto dx() { return Expression<ExprType::Constant, Zero>{}; }
		static constexpr float value = 1;
	};
	using OneExpr = Expression<ExprType::Constant, One>;

	template <>
	struct Expression<ExprType::Constant> {
		constexpr float operator()(float x) { return value; }
		constexpr auto dx() { return Expression<ExprType::Constant, Zero>{}; }
		float value;
	};

	using Constant = Expression<ExprType::Constant>;

	// Variable
	template <>
	struct Expression<ExprType::Variable> {
		constexpr float operator()(float x) { return x; }
		constexpr auto dx() { return Expression<ExprType::Constant, One>{}; }
	};

	using Variable = Expression<ExprType::Variable>;

	// Sum
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	struct Expression<ExprType::Sum, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>> {

		constexpr float operator()(float x) { return lhs(x) + rhs(x); }
		constexpr auto dx() { return lhs.dx() + rhs.dx(); }

		Expression<exprType1, Ts1...> lhs;
		Expression<exprType2, Ts2...> rhs;
	};

	// sum of expressions
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	constexpr auto operator+(const Expression<exprType1, Ts1...>& lhs,
							 const Expression<exprType2, Ts2...>& rhs) {
		using TypeLHS = Expression<exprType1, Ts1...>;
		using TypeRHS = Expression<exprType2, Ts2...>;

		if constexpr (std::is_same_v<TypeLHS, ZeroExpr>) return rhs;
		else if constexpr (std::is_same_v<TypeRHS, ZeroExpr>) return lhs;
		else if constexpr (exprType1 == ExprType::Constant && exprType2 == ExprType::Constant) {
			return Constant{lhs.value + rhs.value};
		}
		else {
			return Expression<ExprType::Sum, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>>{lhs, rhs};
		}
	}

	// Difference
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	struct Expression<ExprType::Difference, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>> {

		constexpr float operator()(float x) { return lhs(x) - rhs(x); }
		constexpr auto dx() { return lhs.dx() - rhs.dx(); }

		Expression<exprType1, Ts1...> lhs;
		Expression<exprType2, Ts2...> rhs;
	};

	// difference of expressions
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	auto operator-(const Expression<exprType1, Ts1...>& lhs,
				   const Expression<exprType2, Ts2...>& rhs) {
		using TypeLHS = Expression<exprType1, Ts1...>;
		using TypeRHS = Expression<exprType2, Ts2...>;

		if constexpr (std::is_same_v<TypeRHS, ZeroExpr>) return lhs;
		else if constexpr (exprType1 == ExprType::Constant && exprType2 == ExprType::Constant) {
			return Expression<ExprType::Constant>{lhs.value - rhs.value};
		}
		else {
			return Expression<ExprType::Difference, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>>{lhs, rhs};
		}
	}

	// Product
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	struct Expression<ExprType::Product, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>> {

		constexpr float operator()(float x) { return lhs(x) * rhs(x); }
		constexpr auto dx() { return lhs.dx() * rhs + lhs * rhs.dx(); }

		Expression<exprType1, Ts1...> lhs;
		Expression<exprType2, Ts2...> rhs;
	};

	// product of expressions
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	auto operator*(const Expression<exprType1, Ts1...>& lhs,
				   const Expression<exprType2, Ts2...>& rhs) {
		using TypeLHS = Expression<exprType1, Ts1...>;
		using TypeRHS = Expression<exprType2, Ts2...>;

		if constexpr (std::is_same_v<TypeLHS, ZeroExpr> || std::is_same_v<TypeRHS, ZeroExpr>) return ZeroExpr{};
		else if constexpr (std::is_same_v<TypeLHS, OneExpr>) return rhs;
		else if constexpr (std::is_same_v<TypeRHS, OneExpr>) return lhs;
		else if constexpr (exprType1 == ExprType::Constant && exprType2 == ExprType::Constant) {
			return Expression<ExprType::Constant>{lhs.value* rhs.value};
		}
		else {
			return Expression<ExprType::Product, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>>{lhs, rhs};
		}
	}

	// Quotient
	template <ExprType exprType1, typename... Ts1, ExprType exprType2, typename... Ts2>
	struct Expression<ExprType::Quotient, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>> {

		constexpr float operator()(float x) { return lhs(x) / rhs(x); }
		constexpr auto dx() { return (lhs.dx() * rhs - lhs * rhs.dx()) / (rhs * rhs); }

		Expression<exprType1, Ts1...> lhs;
		Expression<exprType2, Ts2...> rhs;
	};

	// quotient of expressions
	template <ExprType exprType1, typename... Ts1,
		ExprType exprType2, typename... Ts2>
	auto operator/(const Expression<exprType1, Ts1...>& lhs,
				   const Expression<exprType2, Ts2...>& rhs) {
		using TypeLHS = Expression<exprType1, Ts1...>;
		using TypeRHS = Expression<exprType2, Ts2...>;

		if constexpr (std::is_same_v<TypeRHS, ZeroExpr>) return ZeroExpr{};	// todo: fix this
		else if constexpr (std::is_same_v<TypeLHS, ZeroExpr>) return ZeroExpr{};
		else if constexpr (std::is_same_v<TypeRHS, OneExpr>) return lhs;
		else if constexpr (exprType1 == ExprType::Constant && exprType2 == ExprType::Constant) {
			return Expression<ExprType::Constant>{lhs.value / rhs.value};
		}
		else {
			return Expression<ExprType::Quotient, Expression<exprType1, Ts1...>, Expression<exprType2, Ts2...>>{lhs, rhs};
		}
	}

	// global operators with floats
	template <ExprType exprType, typename... Ts>
	constexpr auto operator+(const Expression<exprType, Ts...>& lhs, float rhs) { return lhs + Constant{rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator+(float lhs, const Expression<exprType, Ts...>& rhs) { return Constant{lhs} + rhs; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator-(const Expression<exprType, Ts...>& lhs, float rhs) { return lhs + Constant{-rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator-(float lhs, const Expression<exprType, Ts...>& rhs) { return Constant{lhs} - rhs; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator*(const Expression<exprType, Ts...>& lhs, float rhs) { return lhs * Constant{rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator*(float lhs, const Expression<exprType, Ts...>& rhs) { return Constant{lhs} *rhs; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator/(const Expression<exprType, Ts...>& lhs, float rhs) { return lhs / Constant{rhs}; }
	template <ExprType exprType, typename... Ts>
	constexpr auto operator/(float lhs, const Expression<exprType, Ts...>& rhs) { return Constant{lhs} / rhs; }



}



