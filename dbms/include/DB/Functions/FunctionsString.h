﻿#pragma once

#include <Poco/UTF8Encoding.h>
#include <Poco/Unicode.h>

#include <DB/Core/FieldVisitors.h>

#include <ext/range.hpp>
#include <DB/Columns/ColumnArray.h>
#include <DB/Columns/ColumnConst.h>
#include <DB/Columns/ColumnFixedString.h>
#include <DB/Columns/ColumnString.h>
#include <DB/DataTypes/DataTypeArray.h>
#include <DB/DataTypes/DataTypeFixedString.h>
#include <DB/DataTypes/DataTypeString.h>
#include <DB/DataTypes/DataTypesNumberFixed.h>
#include <DB/Functions/IFunction.h>

#if __SSE2__
#include <emmintrin.h>
#endif


namespace DB
{
/** Функции работы со строками:
  *
  * length, empty, notEmpty,
  * concat, substring, lower, upper, reverse
  * lengthUTF8, substringUTF8, lowerUTF8, upperUTF8, reverseUTF8
  *
  * s				-> UInt8:	empty, notEmpty
  * s 				-> UInt64: 	length, lengthUTF8
  * s 				-> s:		lower, upper, lowerUTF8, upperUTF8, reverse, reverseUTF8
  * s, s 			-> s: 		concat
  * s, c1, c2 		-> s:		substring, substringUTF8
  * s, c1, c2, s2	-> s:		replace, replaceUTF8
  *
  * Функции поиска строк и регулярных выражений расположены отдельно.
  * Функции работы с URL расположены отдельно.
  * Функции кодирования строк, конвертации в другие типы расположены отдельно.
  *
  * Функции length, empty, notEmpty, reverse также работают с массивами.
  */


template <bool negative = false>
struct EmptyImpl
{
	static void vector(const ColumnString::Chars_t & data, const ColumnString::Offsets_t & offsets, PaddedPODArray<UInt8> & res);

	static void vector_fixed_to_constant(const ColumnString::Chars_t & data, size_t n, UInt8 & res);

	static void vector_fixed_to_vector(const ColumnString::Chars_t & data, size_t n, PaddedPODArray<UInt8> & res);

	static void constant(const std::string & data, UInt8 & res);

	static void array(const ColumnString::Offsets_t & offsets, PaddedPODArray<UInt8> & res);

	static void constant_array(const Array & data, UInt8 & res);
};


/** Вычисляет длину строки в байтах.
  */
struct LengthImpl
{
	static void vector(const ColumnString::Chars_t & data, const ColumnString::Offsets_t & offsets, PaddedPODArray<UInt64> & res);

	static void vector_fixed_to_constant(const ColumnString::Chars_t & data, size_t n, UInt64 & res)
	{
		res = n;
	}

	static void vector_fixed_to_vector(const ColumnString::Chars_t & data, size_t n, PaddedPODArray<UInt64> & res);

	static void constant(const std::string & data, UInt64 & res);

	static void array(const ColumnString::Offsets_t & offsets, PaddedPODArray<UInt64> & res);

	static void constant_array(const Array & data, UInt64 & res);
};


/** Если строка представляет собой текст в кодировке UTF-8, то возвращает длину текста в кодовых точках.
  * (не в символах: длина текста "ё" может быть как 1, так и 2, в зависимости от нормализации)
  * Иначе - поведение не определено.
  */
struct LengthUTF8Impl
{
	static void vector(const ColumnString::Chars_t & data, const ColumnString::Offsets_t & offsets, PaddedPODArray<UInt64> & res);

	static void vector_fixed_to_constant(const ColumnString::Chars_t & data, size_t n, UInt64 & res);

	static void vector_fixed_to_vector(const ColumnString::Chars_t & data, size_t n, PaddedPODArray<UInt64> & res);

	static void constant(const std::string & data, UInt64 & res);

	static void array(const ColumnString::Offsets_t & offsets, PaddedPODArray<UInt64> & res);

	static void constant_array(const Array & data, UInt64 & res);
};


template <char not_case_lower_bound, char not_case_upper_bound>
struct LowerUpperImpl
{
	static void vector(const ColumnString::Chars_t & data,
		const ColumnString::Offsets_t & offsets,
		ColumnString::Chars_t & res_data,
		ColumnString::Offsets_t & res_offsets);

	static void vector_fixed(const ColumnString::Chars_t & data, size_t n, ColumnString::Chars_t & res_data);

	static void constant(const std::string & data, std::string & res_data);

private:
	static void array(const UInt8 * src, const UInt8 * src_end, UInt8 * dst);
};


/// xor or do nothing
template <bool>
UInt8 xor_or_identity(const UInt8 c, const int mask)
{
	return c ^ mask;
};
template <>
inline UInt8 xor_or_identity<false>(const UInt8 c, const int)
{
	return c;
}

/// It is caller's responsibility to ensure the presence of a valid cyrillic sequence in array
template <bool to_lower>
inline void UTF8CyrillicToCase(const UInt8 *& src, const UInt8 * const src_end, UInt8 *& dst);

/** Если строка содержит текст в кодировке UTF-8 - перевести его в нижний (верхний) регистр.
  * Замечание: предполагается, что после перевода символа в другой регистр,
  *  длина его мультибайтовой последовательности в UTF-8 не меняется.
  * Иначе - поведение не определено.
  */
template <char not_case_lower_bound,
	char not_case_upper_bound,
	int to_case(int),
	void cyrillic_to_case(const UInt8 *&, const UInt8 *, UInt8 *&)>
struct LowerUpperUTF8Impl
{
	static void vector(const ColumnString::Chars_t & data,
		const ColumnString::Offsets_t & offsets,
		ColumnString::Chars_t & res_data,
		ColumnString::Offsets_t & res_offsets);

	static void vector_fixed(const ColumnString::Chars_t & data, size_t n, ColumnString::Chars_t & res_data);

	static void constant(const std::string & data, std::string & res_data);

	/** Converts a single code point starting at `src` to desired case, storing result starting at `dst`.
	 *	`src` and `dst` are incremented by corresponding sequence lengths. */
	static void toCase(const UInt8 *& src, const UInt8 * const src_end, UInt8 *& dst);

private:
	static constexpr auto ascii_upper_bound = '\x7f';
	static constexpr auto flip_case_mask = 'A' ^ 'a';

	static void array(const UInt8 * src, const UInt8 * src_end, UInt8 * dst);
};


/** Разворачивает строку в байтах.
  */
struct ReverseImpl
{
	static void vector(const ColumnString::Chars_t & data,
		const ColumnString::Offsets_t & offsets,
		ColumnString::Chars_t & res_data,
		ColumnString::Offsets_t & res_offsets);

	static void vector_fixed(const ColumnString::Chars_t & data, size_t n, ColumnString::Chars_t & res_data);

	static void constant(const std::string & data, std::string & res_data);
};


/** Разворачивает последовательность кодовых точек в строке в кодировке UTF-8.
  * Результат может не соответствовать ожидаемому, так как модифицирующие кодовые точки (например, диакритика) могут примениться не к тем символам.
  * Если строка не в кодировке UTF-8, то поведение не определено.
  */
struct ReverseUTF8Impl
{
	static void vector(const ColumnString::Chars_t & data,
		const ColumnString::Offsets_t & offsets,
		ColumnString::Chars_t & res_data,
		ColumnString::Offsets_t & res_offsets);

	static void vector_fixed(const ColumnString::Chars_t & data, size_t n, ColumnString::Chars_t & res_data);

	static void constant(const std::string & data, std::string & res_data);
};


/** Выделяет подстроку в строке, как последовательности байт.
  */
struct SubstringImpl
{
	static void vector(const ColumnString::Chars_t & data,
		const ColumnString::Offsets_t & offsets,
		size_t start,
		size_t length,
		ColumnString::Chars_t & res_data,
		ColumnString::Offsets_t & res_offsets);

	static void vector_fixed(const ColumnString::Chars_t & data,
		size_t n,
		size_t start,
		size_t length,
		ColumnString::Chars_t & res_data,
		ColumnString::Offsets_t & res_offsets);

	static void constant(const std::string & data, size_t start, size_t length, std::string & res_data);
};


/** Если строка в кодировке UTF-8, то выделяет в ней подстроку кодовых точек.
  * Иначе - поведение не определено.
  */
struct SubstringUTF8Impl
{
	static void vector(const ColumnString::Chars_t & data,
		const ColumnString::Offsets_t & offsets,
		size_t start,
		size_t length,
		ColumnString::Chars_t & res_data,
		ColumnString::Offsets_t & res_offsets);

	static void vector_fixed(const ColumnString::Chars_t & data,
		ColumnString::Offset_t n,
		size_t start,
		size_t length,
		ColumnString::Chars_t & res_data,
		ColumnString::Offsets_t & res_offsets);

	static void constant(const std::string & data, size_t start, size_t length, std::string & res_data);
};


template <typename Impl, typename Name, typename ResultType>
class FunctionStringOrArrayToT : public IFunction
{
public:
	static constexpr auto name = Name::name;
	static FunctionPtr create(const Context & context);

	/// Получить имя функции.
	String getName() const override
	{
		return name;
	}

	size_t getNumberOfArguments() const override
	{
		return 1;
	}

	/// Получить тип результата по типам аргументов. Если функция неприменима для данных аргументов - кинуть исключение.
	DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override;

	/// Выполнить функцию над блоком.
	void executeImpl(Block & block, const ColumnNumbers & arguments, size_t result) override;
};


template <typename Impl, typename Name, bool is_injective = false>
class FunctionStringToString : public IFunction
{
public:
	static constexpr auto name = Name::name;
	static FunctionPtr create(const Context & context)
	{
		return std::make_shared<FunctionStringToString>();
	}

	/// Получить имя функции.
	String getName() const override
	{
		return name;
	}

	size_t getNumberOfArguments() const override
	{
		return 1;
	}
	bool isInjective(const Block &) override
	{
		return is_injective;
	}

	/// Получить тип результата по типам аргументов. Если функция неприменима для данных аргументов - кинуть исключение.
	DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override
	{
		if (!typeid_cast<const DataTypeString *>(&*arguments[0]) && !typeid_cast<const DataTypeFixedString *>(&*arguments[0]))
			throw Exception(
				"Illegal type " + arguments[0]->getName() + " of argument of function " + getName(), ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);

		return arguments[0]->clone();
	}

	/// Выполнить функцию над блоком.
	void executeImpl(Block & block, const ColumnNumbers & arguments, size_t result) override
	{
		const ColumnPtr column = block.safeGetByPosition(arguments[0]).column;
		if (const ColumnString * col = typeid_cast<const ColumnString *>(&*column))
		{
			std::shared_ptr<ColumnString> col_res = std::make_shared<ColumnString>();
			block.safeGetByPosition(result).column = col_res;
			Impl::vector(col->getChars(), col->getOffsets(), col_res->getChars(), col_res->getOffsets());
		}
		else if (const ColumnFixedString * col = typeid_cast<const ColumnFixedString *>(&*column))
		{
			auto col_res = std::make_shared<ColumnFixedString>(col->getN());
			block.safeGetByPosition(result).column = col_res;
			Impl::vector_fixed(col->getChars(), col->getN(), col_res->getChars());
		}
		else if (const ColumnConstString * col = typeid_cast<const ColumnConstString *>(&*column))
		{
			String res;
			Impl::constant(col->getData(), res);
			auto col_res = std::make_shared<ColumnConstString>(col->size(), res);
			block.safeGetByPosition(result).column = col_res;
		}
		else
			throw Exception(
				"Illegal column " + block.safeGetByPosition(arguments[0]).column->getName() + " of argument of function " + getName(),
				ErrorCodes::ILLEGAL_COLUMN);
	}
};


/// Также работает над массивами.
class FunctionReverse : public IFunction
{
public:
	static constexpr auto name = "reverse";
	static FunctionPtr create(const Context & context)
	{
		return std::make_shared<FunctionReverse>();
	}

	/// Получить имя функции.
	String getName() const override;

	size_t getNumberOfArguments() const override
	{
		return 1;
	}
	bool isInjective(const Block &) override
	{
		return true;
	}
	DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override;

	/// Выполнить функцию над блоком.
	void executeImpl(Block & block, const ColumnNumbers & arguments, size_t result) override;
};


template <typename Name, bool is_injective>
class ConcatImpl : public IFunction
{
public:
	static constexpr auto name = Name::name;
	static FunctionPtr create(const Context & context)
	{
		return std::make_shared<ConcatImpl>();
	}

	/// Получить имя функции.
	String getName() const override
	{
		return name;
	}

	bool isVariadic() const override
	{
		return true;
	}
	size_t getNumberOfArguments() const override
	{
		return 0;
	}
	bool isInjective(const Block &) override
	{
		return is_injective;
	}

	/// Получить тип результата по типам аргументов. Если функция неприменима для данных аргументов - кинуть исключение.
	DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override
	{
		if (arguments.size() < 2)
			throw Exception("Number of arguments for function " + getName() + " doesn't match: passed " + toString(arguments.size())
					+ ", should be at least 2.",
				ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH);

		for (const auto arg_idx : ext::range(0, arguments.size()))
		{
			const auto arg = arguments[arg_idx].get();
			if (!typeid_cast<const DataTypeString *>(arg) && !typeid_cast<const DataTypeFixedString *>(arg))
				throw Exception{
					"Illegal type " + arg->getName() + " of argument " + std::to_string(arg_idx + 1) + " of function " + getName(),
					ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT};
		}

		return std::make_shared<DataTypeString>();
	}

	void executeImpl(Block & block, const ColumnNumbers & arguments, const size_t result) override
	{
		if (arguments.size() == 2)
			executeBinary(block, arguments, result);
		else
			executeNAry(block, arguments, result);
	}

private:
	enum class InstructionType : UInt8
	{
		COPY_STRING,
		COPY_FIXED_STRING,
		COPY_CONST_STRING
	};

	/// column pointer augmented with offset (current offset String/FixedString, unused for Const<String>)
	using ColumnAndOffset = std::pair<const IColumn *, IColumn::Offset_t>;
	/// InstructionType is being stored to allow using static_cast safely
	using Instruction = std::pair<InstructionType, ColumnAndOffset>;
	using Instructions = std::vector<Instruction>;

	/** calculate total length of resulting strings (without terminating nulls), determine whether all input
	  *	strings are constant, assemble instructions
	  */
	Instructions getInstructions(const Block & block, const ColumnNumbers & arguments, size_t & out_length, bool & out_const)
	{
		Instructions result{};
		result.reserve(arguments.size());

		out_length = 0;
		out_const = true;

		size_t rows{};
		for (const auto arg_pos : arguments)
		{
			const auto column = block.safeGetByPosition(arg_pos).column.get();

			if (const auto col = typeid_cast<const ColumnString *>(column))
			{
				/** ColumnString stores strings with terminating null character
				  *  which should not be copied, therefore the decrease of total size by
				  *	the number of terminating nulls
				  */
				rows = col->size();
				out_length += col->getChars().size() - col->getOffsets().size();
				out_const = false;

				result.emplace_back(InstructionType::COPY_STRING, ColumnAndOffset{col, 0});
			}
			else if (const auto col = typeid_cast<const ColumnFixedString *>(column))
			{
				rows = col->size();
				out_length += col->getChars().size();
				out_const = false;

				result.emplace_back(InstructionType::COPY_FIXED_STRING, ColumnAndOffset{col, 0});
			}
			else if (const auto col = typeid_cast<const ColumnConstString *>(column))
			{
				rows = col->size();
				out_length += col->getData().size() * col->size();
				out_const = out_const && true;

				result.emplace_back(InstructionType::COPY_CONST_STRING, ColumnAndOffset{col, 0});
			}
			else
				throw Exception(
					"Illegal column " + column->getName() + " of argument of function " + getName(), ErrorCodes::ILLEGAL_COLUMN);
		}

		if (out_const && rows)
			out_length /= rows;

		return result;
	}

	void executeBinary(Block & block, const ColumnNumbers & arguments, const size_t result)
	{
		const IColumn * c0 = block.safeGetByPosition(arguments[0]).column.get();
		const IColumn * c1 = block.safeGetByPosition(arguments[1]).column.get();

		const ColumnString * c0_string = typeid_cast<const ColumnString *>(c0);
		const ColumnString * c1_string = typeid_cast<const ColumnString *>(c1);
		const ColumnFixedString * c0_fixed_string = typeid_cast<const ColumnFixedString *>(c0);
		const ColumnFixedString * c1_fixed_string = typeid_cast<const ColumnFixedString *>(c1);
		const ColumnConstString * c0_const = typeid_cast<const ColumnConstString *>(c0);
		const ColumnConstString * c1_const = typeid_cast<const ColumnConstString *>(c1);

		/// Результат - const string
		if (c0_const && c1_const)
		{
			auto c_res = std::make_shared<ColumnConstString>(c0_const->size(), "");
			block.safeGetByPosition(result).column = c_res;
			constant_constant(c0_const->getData(), c1_const->getData(), c_res->getData());
		}
		else
		{
			auto c_res = std::make_shared<ColumnString>();
			block.safeGetByPosition(result).column = c_res;
			ColumnString::Chars_t & vec_res = c_res->getChars();
			ColumnString::Offsets_t & offsets_res = c_res->getOffsets();

			if (c0_string && c1_string)
				vector_vector(
					c0_string->getChars(), c0_string->getOffsets(), c1_string->getChars(), c1_string->getOffsets(), vec_res, offsets_res);
			else if (c0_string && c1_fixed_string)
				vector_fixed_vector(c0_string->getChars(),
					c0_string->getOffsets(),
					c1_fixed_string->getChars(),
					c1_fixed_string->getN(),
					vec_res,
					offsets_res);
			else if (c0_string && c1_const)
				vector_constant(c0_string->getChars(), c0_string->getOffsets(), c1_const->getData(), vec_res, offsets_res);
			else if (c0_fixed_string && c1_string)
				fixed_vector_vector(c0_fixed_string->getChars(),
					c0_fixed_string->getN(),
					c1_string->getChars(),
					c1_string->getOffsets(),
					vec_res,
					offsets_res);
			else if (c0_const && c1_string)
				constant_vector(c0_const->getData(), c1_string->getChars(), c1_string->getOffsets(), vec_res, offsets_res);
			else if (c0_fixed_string && c1_fixed_string)
				fixed_vector_fixed_vector(c0_fixed_string->getChars(),
					c0_fixed_string->getN(),
					c1_fixed_string->getChars(),
					c1_fixed_string->getN(),
					vec_res,
					offsets_res);
			else if (c0_fixed_string && c1_const)
				fixed_vector_constant(c0_fixed_string->getChars(), c0_fixed_string->getN(), c1_const->getData(), vec_res, offsets_res);
			else if (c0_const && c1_fixed_string)
				constant_fixed_vector(c0_const->getData(), c1_fixed_string->getChars(), c1_fixed_string->getN(), vec_res, offsets_res);
			else
				throw Exception("Illegal columns " + block.safeGetByPosition(arguments[0]).column->getName() + " and "
						+ block.safeGetByPosition(arguments[1]).column->getName()
						+ " of arguments of function "
						+ getName(),
					ErrorCodes::ILLEGAL_COLUMN);
		}
	}

	void executeNAry(Block & block, const ColumnNumbers & arguments, const size_t result)
	{
		const auto size = block.rows();
		std::size_t result_length{};
		bool result_is_const{};
		auto instrs = getInstructions(block, arguments, result_length, result_is_const);

		if (result_is_const)
		{
			const auto out = std::make_shared<ColumnConstString>(size, "");
			block.safeGetByPosition(result).column = out;

			auto & data = out->getData();
			data.reserve(result_length);

			for (const auto & instr : instrs)
				data += static_cast<const ColumnConst<String> *>(instr.second.first)->getData();
		}
		else
		{
			const auto out = std::make_shared<ColumnString>();
			block.safeGetByPosition(result).column = out;

			auto & out_data = out->getChars();
			out_data.resize(result_length + size);

			auto & out_offsets = out->getOffsets();
			out_offsets.resize(size);

			std::size_t out_offset{};

			for (const auto row : ext::range(0, size))
			{
				for (auto & instr : instrs)
				{
					switch (instr.first)
					{
						case InstructionType::COPY_STRING:
						{
							auto & in_offset = instr.second.second;
							const auto col = static_cast<const ColumnString *>(instr.second.first);
							const auto offset = col->getOffsets()[row];
							const auto length = offset - in_offset - 1;

							memcpySmallAllowReadWriteOverflow15(&out_data[out_offset], &col->getChars()[in_offset], length);
							out_offset += length;
							in_offset = offset;
							break;
						}
						case InstructionType::COPY_FIXED_STRING:
						{
							auto & in_offset = instr.second.second;
							const auto col = static_cast<const ColumnFixedString *>(instr.second.first);
							const auto length = col->getN();

							memcpySmallAllowReadWriteOverflow15(&out_data[out_offset], &col->getChars()[in_offset], length);
							out_offset += length;
							in_offset += length;
							break;
						}
						case InstructionType::COPY_CONST_STRING:
						{
							const auto col = static_cast<const ColumnConst<String> *>(instr.second.first);
							const auto & data = col->getData();
							const auto length = data.size();

							memcpy(&out_data[out_offset], data.data(), length);
							out_offset += length;
							break;
						}
						default:
							throw Exception("Unknown InstructionType during execution of function 'concat'", ErrorCodes::LOGICAL_ERROR);
					}
				}

				out_data[out_offset] = '\0';
				out_offsets[row] = ++out_offset;
			}
		}
	}

	static void vector_vector(const ColumnString::Chars_t & a_data,
		const ColumnString::Offsets_t & a_offsets,
		const ColumnString::Chars_t & b_data,
		const ColumnString::Offsets_t & b_offsets,
		ColumnString::Chars_t & c_data,
		ColumnString::Offsets_t & c_offsets)
	{
		size_t size = a_offsets.size();
		c_data.resize(a_data.size() + b_data.size() - size);
		c_offsets.resize(size);

		ColumnString::Offset_t offset = 0;
		ColumnString::Offset_t a_offset = 0;
		ColumnString::Offset_t b_offset = 0;
		for (size_t i = 0; i < size; ++i)
		{
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &a_data[a_offset], a_offsets[i] - a_offset - 1);
			offset += a_offsets[i] - a_offset - 1;
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &b_data[b_offset], b_offsets[i] - b_offset);
			offset += b_offsets[i] - b_offset;

			a_offset = a_offsets[i];
			b_offset = b_offsets[i];

			c_offsets[i] = offset;
		}
	}

	static void vector_fixed_vector(const ColumnString::Chars_t & a_data,
		const ColumnString::Offsets_t & a_offsets,
		const ColumnString::Chars_t & b_data,
		ColumnString::Offset_t b_n,
		ColumnString::Chars_t & c_data,
		ColumnString::Offsets_t & c_offsets)
	{
		size_t size = a_offsets.size();
		c_data.resize(a_data.size() + b_data.size());
		c_offsets.resize(size);

		ColumnString::Offset_t offset = 0;
		ColumnString::Offset_t a_offset = 0;
		ColumnString::Offset_t b_offset = 0;
		for (size_t i = 0; i < size; ++i)
		{
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &a_data[a_offset], a_offsets[i] - a_offset - 1);
			offset += a_offsets[i] - a_offset - 1;
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &b_data[b_offset], b_n);
			offset += b_n;
			c_data[offset] = 0;
			offset += 1;

			a_offset = a_offsets[i];
			b_offset += b_n;

			c_offsets[i] = offset;
		}
	}

	static void vector_constant(const ColumnString::Chars_t & a_data,
		const ColumnString::Offsets_t & a_offsets,
		const std::string & b,
		ColumnString::Chars_t & c_data,
		ColumnString::Offsets_t & c_offsets)
	{
		size_t size = a_offsets.size();
		c_data.resize(a_data.size() + b.size() * size);
		c_offsets.assign(a_offsets);

		for (size_t i = 0; i < size; ++i)
			c_offsets[i] += b.size() * (i + 1);

		ColumnString::Offset_t offset = 0;
		ColumnString::Offset_t a_offset = 0;
		for (size_t i = 0; i < size; ++i)
		{
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &a_data[a_offset], a_offsets[i] - a_offset - 1);
			offset += a_offsets[i] - a_offset - 1;
			memcpy(&c_data[offset], b.data(), b.size() + 1);
			offset += b.size() + 1;

			a_offset = a_offsets[i];
		}
	}

	static void fixed_vector_vector(const ColumnString::Chars_t & a_data,
		ColumnString::Offset_t a_n,
		const ColumnString::Chars_t & b_data,
		const ColumnString::Offsets_t & b_offsets,
		ColumnString::Chars_t & c_data,
		ColumnString::Offsets_t & c_offsets)
	{
		size_t size = b_offsets.size();
		c_data.resize(a_data.size() + b_data.size());
		c_offsets.resize(size);

		ColumnString::Offset_t offset = 0;
		ColumnString::Offset_t a_offset = 0;
		ColumnString::Offset_t b_offset = 0;
		for (size_t i = 0; i < size; ++i)
		{
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &a_data[a_offset], a_n);
			offset += a_n;
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &b_data[b_offset], b_offsets[i] - b_offset);
			offset += b_offsets[i] - b_offset;

			a_offset = a_n;
			b_offset = b_offsets[i];

			c_offsets[i] = offset;
		}
	}

	static void fixed_vector_fixed_vector(const ColumnString::Chars_t & a_data,
		ColumnString::Offset_t a_n,
		const ColumnString::Chars_t & b_data,
		ColumnString::Offset_t b_n,
		ColumnString::Chars_t & c_data,
		ColumnString::Offsets_t & c_offsets)
	{
		size_t size = a_data.size() / a_n;
		c_data.resize(a_data.size() + b_data.size() + size);
		c_offsets.resize(size);

		ColumnString::Offset_t offset = 0;
		for (size_t i = 0; i < size; ++i)
		{
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &a_data[i * a_n], a_n);
			offset += a_n;
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &b_data[i * b_n], b_n);
			offset += b_n;
			c_data[offset] = 0;
			++offset;

			c_offsets[i] = offset;
		}
	}

	static void fixed_vector_constant(const ColumnString::Chars_t & a_data,
		ColumnString::Offset_t a_n,
		const std::string & b,
		ColumnString::Chars_t & c_data,
		ColumnString::Offsets_t & c_offsets)
	{
		size_t size = a_data.size() / a_n;
		ColumnString::Offset_t b_n = b.size();
		c_data.resize(a_data.size() + size * b_n + size);
		c_offsets.resize(size);

		ColumnString::Offset_t offset = 0;
		for (size_t i = 0; i < size; ++i)
		{
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &a_data[i * a_n], a_n);
			offset += a_n;
			memcpy(&c_data[offset], b.data(), b_n);
			offset += b_n;
			c_data[offset] = 0;
			++offset;

			c_offsets[i] = offset;
		}
	}

	static void constant_vector(const std::string & a,
		const ColumnString::Chars_t & b_data,
		const ColumnString::Offsets_t & b_offsets,
		ColumnString::Chars_t & c_data,
		ColumnString::Offsets_t & c_offsets)
	{
		size_t size = b_offsets.size();
		c_data.resize(b_data.size() + a.size() * size);
		c_offsets.assign(b_offsets);

		for (size_t i = 0; i < size; ++i)
			c_offsets[i] += a.size() * (i + 1);

		ColumnString::Offset_t offset = 0;
		ColumnString::Offset_t b_offset = 0;
		for (size_t i = 0; i < size; ++i)
		{
			memcpy(&c_data[offset], a.data(), a.size());
			offset += a.size();
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &b_data[b_offset], b_offsets[i] - b_offset);
			offset += b_offsets[i] - b_offset;

			b_offset = b_offsets[i];
		}
	}

	static void constant_fixed_vector(const std::string & a,
		const ColumnString::Chars_t & b_data,
		ColumnString::Offset_t b_n,
		ColumnString::Chars_t & c_data,
		ColumnString::Offsets_t & c_offsets)
	{
		size_t size = b_data.size() / b_n;
		ColumnString::Offset_t a_n = a.size();
		c_data.resize(size * a_n + b_data.size() + size);
		c_offsets.resize(size);

		ColumnString::Offset_t offset = 0;
		for (size_t i = 0; i < size; ++i)
		{
			memcpy(&c_data[offset], a.data(), a_n);
			offset += a_n;
			memcpySmallAllowReadWriteOverflow15(&c_data[offset], &b_data[i * b_n], b_n);
			offset += b_n;
			c_data[offset] = 0;
			++offset;

			c_offsets[i] = offset;
		}
	}

	static void constant_constant(const std::string & a, const std::string & b, std::string & c)
	{
		c = a + b;
	}
};


template <typename Impl, typename Name>
class FunctionStringNumNumToString : public IFunction
{
public:
	static constexpr auto name = Name::name;
	static FunctionPtr create(const Context & context)
	{
		return std::make_shared<FunctionStringNumNumToString>();
	}

	/// Получить имя функции.
	String getName() const override
	{
		return name;
	}

	size_t getNumberOfArguments() const override
	{
		return 3;
	}

	/// Получить тип результата по типам аргументов. Если функция неприменима для данных аргументов - кинуть исключение.
	DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override
	{
		if (!typeid_cast<const DataTypeString *>(&*arguments[0]) && !typeid_cast<const DataTypeFixedString *>(&*arguments[0]))
			throw Exception(
				"Illegal type " + arguments[0]->getName() + " of argument of function " + getName(), ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);

		if (!arguments[1]->isNumeric() || !arguments[2]->isNumeric())
			throw Exception("Illegal type " + (arguments[1]->isNumeric() ? arguments[2]->getName() : arguments[1]->getName())
					+ " of argument of function "
					+ getName(),
				ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);

		return std::make_shared<DataTypeString>();
	}

	/// Выполнить функцию над блоком.
	void executeImpl(Block & block, const ColumnNumbers & arguments, size_t result) override
	{
		const ColumnPtr column_string = block.safeGetByPosition(arguments[0]).column;
		const ColumnPtr column_start = block.safeGetByPosition(arguments[1]).column;
		const ColumnPtr column_length = block.safeGetByPosition(arguments[2]).column;

		if (!column_start->isConst() || !column_length->isConst())
			throw Exception("2nd and 3rd arguments of function " + getName() + " must be constants.");

		Field start_field = (*block.safeGetByPosition(arguments[1]).column)[0];
		Field length_field = (*block.safeGetByPosition(arguments[2]).column)[0];

		if (start_field.getType() != Field::Types::UInt64 || length_field.getType() != Field::Types::UInt64)
			throw Exception("2nd and 3rd arguments of function " + getName() + " must be non-negative and must have UInt type.");

		UInt64 start = start_field.get<UInt64>();
		UInt64 length = length_field.get<UInt64>();

		if (start == 0)
			throw Exception("Second argument of function substring must be greater than 0.", ErrorCodes::ARGUMENT_OUT_OF_BOUND);

		/// Otherwise may lead to overflow and pass bounds check inside inner loop.
		if (start >= 0x8000000000000000ULL || length >= 0x8000000000000000ULL)
			throw Exception("Too large values of 2nd or 3rd argument provided for function substring.", ErrorCodes::ARGUMENT_OUT_OF_BOUND);

		if (const ColumnString * col = typeid_cast<const ColumnString *>(&*column_string))
		{
			std::shared_ptr<ColumnString> col_res = std::make_shared<ColumnString>();
			block.safeGetByPosition(result).column = col_res;
			Impl::vector(col->getChars(), col->getOffsets(), start, length, col_res->getChars(), col_res->getOffsets());
		}
		else if (const ColumnFixedString * col = typeid_cast<const ColumnFixedString *>(&*column_string))
		{
			std::shared_ptr<ColumnString> col_res = std::make_shared<ColumnString>();
			block.safeGetByPosition(result).column = col_res;
			Impl::vector_fixed(col->getChars(), col->getN(), start, length, col_res->getChars(), col_res->getOffsets());
		}
		else if (const ColumnConstString * col = typeid_cast<const ColumnConstString *>(&*column_string))
		{
			String res;
			Impl::constant(col->getData(), start, length, res);
			auto col_res = std::make_shared<ColumnConstString>(col->size(), res);
			block.safeGetByPosition(result).column = col_res;
		}
		else
			throw Exception(
				"Illegal column " + block.safeGetByPosition(arguments[0]).column->getName() + " of first argument of function " + getName(),
				ErrorCodes::ILLEGAL_COLUMN);
	}
};


class FunctionAppendTrailingCharIfAbsent : public IFunction
{
public:
	static constexpr auto name = "appendTrailingCharIfAbsent";
	static FunctionPtr create(const Context & context)
	{
		return std::make_shared<FunctionAppendTrailingCharIfAbsent>();
	}

	String getName() const override;

private:
	size_t getNumberOfArguments() const override
	{
		return 2;
	}
	DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override;
	void executeImpl(Block & block, const ColumnNumbers & arguments, const size_t result) override;
};


struct NameEmpty
{
	static constexpr auto name = "empty";
};
struct NameNotEmpty
{
	static constexpr auto name = "notEmpty";
};
struct NameLength
{
	static constexpr auto name = "length";
};
struct NameLengthUTF8
{
	static constexpr auto name = "lengthUTF8";
};
struct NameLower
{
	static constexpr auto name = "lower";
};
struct NameUpper
{
	static constexpr auto name = "upper";
};
struct NameLowerUTF8
{
	static constexpr auto name = "lowerUTF8";
};
struct NameUpperUTF8
{
	static constexpr auto name = "upperUTF8";
};
struct NameReverseUTF8
{
	static constexpr auto name = "reverseUTF8";
};
struct NameSubstring
{
	static constexpr auto name = "substring";
};
struct NameSubstringUTF8
{
	static constexpr auto name = "substringUTF8";
};
struct NameConcat
{
	static constexpr auto name = "concat";
};
struct NameConcatAssumeInjective
{
	static constexpr auto name = "concatAssumeInjective";
};

using FunctionEmpty = FunctionStringOrArrayToT<EmptyImpl<false>, NameEmpty, UInt8>;
using FunctionNotEmpty = FunctionStringOrArrayToT<EmptyImpl<true>, NameNotEmpty, UInt8>;
using FunctionLength = FunctionStringOrArrayToT<LengthImpl, NameLength, UInt64>;
using FunctionLengthUTF8 = FunctionStringOrArrayToT<LengthUTF8Impl, NameLengthUTF8, UInt64>;
using FunctionLower = FunctionStringToString<LowerUpperImpl<'A', 'Z'>, NameLower>;
using FunctionUpper = FunctionStringToString<LowerUpperImpl<'a', 'z'>, NameUpper>;
typedef FunctionStringToString<LowerUpperUTF8Impl<'A', 'Z', Poco::Unicode::toLower, UTF8CyrillicToCase<true>>, NameLowerUTF8>
	FunctionLowerUTF8;
typedef FunctionStringToString<LowerUpperUTF8Impl<'a', 'z', Poco::Unicode::toUpper, UTF8CyrillicToCase<false>>, NameUpperUTF8>
	FunctionUpperUTF8;
using FunctionReverseUTF8 = FunctionStringToString<ReverseUTF8Impl, NameReverseUTF8, true>;
using FunctionSubstring = FunctionStringNumNumToString<SubstringImpl, NameSubstring>;
using FunctionSubstringUTF8 = FunctionStringNumNumToString<SubstringUTF8Impl, NameSubstringUTF8>;
using FunctionConcat = ConcatImpl<NameConcat, false>;
using FunctionConcatAssumeInjective = ConcatImpl<NameConcatAssumeInjective, true>;
}
