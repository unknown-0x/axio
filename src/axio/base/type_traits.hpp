#ifndef AXIO_BASE_TYPE_TRAITS_HPP_
#define AXIO_BASE_TYPE_TRAITS_HPP_

#include "types.hpp"

#include <functional>
#include <type_traits>

namespace axio {
/**
 * @brief Holds a compile-time constant value.
 *
 * Similar to std::integral_constant.
 *
 * @tparam T Type of the stored value.
 * @tparam VALUE Compile-time constant value to store.
 *
 * ```cpp
 * using Five = IntegralConstant<int, 5>;
 *
 * static_assert(Five::value == 5);
 * static_assert(Five{}() == 5);
 * ```
 */
template <typename T, T VALUE>
struct IntegralConstant {
  static constexpr T value = VALUE;
  using ValueType = T;

  constexpr operator ValueType() const noexcept { return value; }
  constexpr ValueType operator()() const noexcept { return value; }
};

/**
 * @brief Compile-time boolean constant.
 * @tparam VALUE Boolean value to store.
 */
template <Bool VALUE>
using BoolConstant = IntegralConstant<Bool, VALUE>;
/// @brief Compile-time constant equal to `true`.
using TrueType = BoolConstant<true>;
/// @brief Compile-time constant equal to `false`.
using FalseType = BoolConstant<false>;

/**
 * @brief Checks whether T is void (ignoring cv-qualifiers).
 * @tparam T Type to check.
 * @see std::is_void
 */
template <typename T>
struct IsVoid : BoolConstant<std::is_void<T>::value> {};
/// @brief Value form of IsVoid.
template <typename T>
inline constexpr auto IsVoid_V = IsVoid<T>::value;

/**
 * @brief Checks whether T is std::nullptr_t.
 * @tparam T Type to check.
 * @see std::is_null_pointer
 */
template <typename T>
struct IsNullPointer : BoolConstant<std::is_null_pointer<T>::value> {};
/// @brief Value form of IsNullPointer.
template <typename T>
inline constexpr auto IsNullPointer_V = IsNullPointer<T>::value;

/**
 * @brief Checks whether T is an integral type.
 * @tparam T Type to check.
 * @see std::is_integral
 */
template <typename T>
struct IsIntegral : BoolConstant<std::is_integral<T>::value> {};
/// @brief Value form of IsIntegral.
template <typename T>
inline constexpr auto IsIntegral_V = IsIntegral<T>::value;

/**
 * @brief Checks whether T is a floating-point type.
 * @tparam T Type to check.
 * @see std::is_floating_point
 */
template <typename T>
struct IsFloatingPoint : BoolConstant<std::is_floating_point<T>::value> {};
/// @brief Value form of IsFloatingPoint.
template <typename T>
inline constexpr auto IsFloatingPoint_V = IsFloatingPoint<T>::value;

/**
 * @brief Checks whether T is an array type (bounded or unbounded).
 * @tparam T Type to check.
 * @see std::is_array
 */
template <typename T>
struct IsArray : BoolConstant<std::is_array<T>::value> {};
/// @brief Value form of IsArray.
template <typename T>
inline constexpr auto IsArray_V = IsArray<T>::value;

/**
 * @brief Checks whether T is an enumeration type.
 * @tparam T Type to check.
 * @see std::is_enum
 */
template <typename T>
struct IsEnum : BoolConstant<std::is_enum<T>::value> {};
/// @brief Value form of IsEnum.
template <typename T>
inline constexpr auto IsEnum_V = IsEnum<T>::value;

/**
 * @brief Checks whether T is a union type.
 * @tparam T Type to check.
 * @see std::is_union
 */
template <typename T>
struct IsUnion : BoolConstant<std::is_union<T>::value> {};
/// @brief Value form of IsUnion.
template <typename T>
inline constexpr auto IsUnion_V = IsUnion<T>::value;

/**
 * @brief Checks whether T is a non-union class type.
 * @tparam T Type to check.
 * @see std::is_class
 */
template <typename T>
struct IsClass : BoolConstant<std::is_class<T>::value> {};
/// @brief Value form of IsClass.
template <typename T>
inline constexpr auto IsClass_V = IsClass<T>::value;

/**
 * @brief Checks whether T is a function type.
 * @tparam T Type to check.
 * @see std::is_function
 */
template <typename T>
struct IsFunction : BoolConstant<std::is_function<T>::value> {};
/// @brief Value form of IsFunction.
template <typename T>
inline constexpr auto IsFunction_V = IsFunction<T>::value;

/**
 * @brief Checks whether T is a pointer type.
 * @tparam T Type to check.
 * @see std::is_pointer
 */
template <typename T>
struct IsPointer : BoolConstant<std::is_pointer<T>::value> {};
/// @brief Value form of IsPointer.
template <typename T>
inline constexpr auto IsPointer_V = IsPointer<T>::value;

/**
 * @brief Checks whether T is an lvalue reference type.
 * @tparam T Type to check.
 * @see std::is_lvalue_reference
 */
template <typename T>
struct IsLvalueReference : BoolConstant<std::is_lvalue_reference<T>::value> {};
/// @brief Value form of IsLvalueReference.
template <typename T>
inline constexpr auto IsLvalueReference_V = IsLvalueReference<T>::value;

/**
 * @brief Checks whether T is an rvalue reference type.
 * @tparam T Type to check.
 * @see std::is_rvalue_reference
 */
template <typename T>
struct IsRvalueReference : BoolConstant<std::is_rvalue_reference<T>::value> {};
/// @brief Value form of IsRvalueReference.
template <typename T>
inline constexpr auto IsRvalueReference_V = IsRvalueReference<T>::value;

/**
 * @brief Checks whether T is a pointer to non-static data member.
 * @tparam T Type to check.
 * @see std::is_member_object_pointer
 */
template <typename T>
struct IsMemberObjectPointer
    : BoolConstant<std::is_member_object_pointer<T>::value> {};
/// @brief Value form of IsMemberObjectPointer.
template <typename T>
inline constexpr auto IsMemberObjectPointer_V = IsMemberObjectPointer<T>::value;

/**
 * @brief Checks whether T is a pointer to non-static member function.
 * @tparam T Type to check.
 * @see std::is_member_function_pointer
 */
template <typename T>
struct IsMemberFunctionPointer
    : BoolConstant<std::is_member_function_pointer<T>::value> {};
/// @brief Value form of IsMemberFunctionPointer.
template <typename T>
inline constexpr auto IsMemberFunctionPointer_V =
    IsMemberFunctionPointer<T>::value;

/**
 * @brief Checks whether T is a fundamental type.
 * @tparam T Type to check.
 * @see std::is_fundamental
 */
template <typename T>
struct IsFundamental : BoolConstant<std::is_fundamental<T>::value> {};
/// @brief Value form of IsFundamental.
template <typename T>
inline constexpr auto IsFundamental_V = IsFundamental<T>::value;

/**
 * @brief Checks whether T is an arithmetic type.
 * @tparam T Type to check.
 * @see std::is_arithmetic
 */
template <typename T>
struct IsArithmetic : BoolConstant<std::is_arithmetic<T>::value> {};
/// @brief Value form of IsArithmetic.
template <typename T>
inline constexpr auto IsArithmetic_V = IsArithmetic<T>::value;

/**
 * @brief Checks whether T is a scalar type.
 * @tparam T Type to check.
 * @see std::is_scalar
 */
template <typename T>
struct IsScalar : BoolConstant<std::is_scalar<T>::value> {};
/// @brief Value form of IsScalar.
template <typename T>
inline constexpr auto IsScalar_V = IsScalar<T>::value;

/**
 * @brief Checks whether T is an object type.
 * @tparam T Type to check.
 * @see std::is_object
 */
template <typename T>
struct IsObject : BoolConstant<std::is_object<T>::value> {};
/// @brief Value form of IsObject.
template <typename T>
inline constexpr auto IsObject_V = IsObject<T>::value;

/**
 * @brief Checks whether T is a compound type (not fundamental).
 * @tparam T Type to check.
 * @see std::is_compound
 */
template <typename T>
struct IsCompound : BoolConstant<std::is_compound<T>::value> {};
/// @brief Value form of IsCompound.
template <typename T>
inline constexpr auto IsCompound_V = IsCompound<T>::value;

/**
 * @brief Checks whether T is an lvalue or rvalue reference type.
 * @tparam T Type to check.
 * @see std::is_reference
 */
template <typename T>
struct IsReference : BoolConstant<std::is_reference<T>::value> {};
/// @brief Value form of IsReference.
template <typename T>
inline constexpr auto IsReference_V = IsReference<T>::value;

/**
 * @brief Checks whether T is a pointer to a non-static member (object or
 * function).
 * @tparam T Type to check.
 * @see std::is_member_pointer
 */
template <typename T>
struct IsMemberPointer : BoolConstant<std::is_member_pointer<T>::value> {};
/// @brief Value form of IsMemberPointer.
template <typename T>
inline constexpr auto IsMemberPointer_V = IsMemberPointer<T>::value;

/**
 * @brief Checks whether T is const-qualified.
 * @tparam T Type to check.
 * @see std::is_const
 */
template <typename T>
struct IsConst : BoolConstant<std::is_const<T>::value> {};
/// @brief Value form of IsConst.
template <typename T>
inline constexpr auto IsConst_V = IsConst<T>::value;

/**
 * @brief Checks whether T is volatile-qualified.
 * @tparam T Type to check.
 * @see std::is_volatile
 */
template <typename T>
struct IsVolatile : BoolConstant<std::is_volatile<T>::value> {};
/// @brief Value form of IsVolatile.
template <typename T>
inline constexpr auto IsVolatile_V = IsVolatile<T>::value;

/**
 * @brief Checks whether T can be copied using a bitwise (trivial) copy.
 * @tparam T Type to check.
 * @see std::is_trivially_copyable
 */
template <typename T>
struct IsTriviallyCopyable
    : BoolConstant<std::is_trivially_copyable<T>::value> {};
/// @brief Value form of IsTriviallyCopyable.
template <typename T>
inline constexpr auto IsTriviallyCopyable_V = IsTriviallyCopyable<T>::value;

/**
 * @brief Checks whether T has standard layout.
 * @tparam T Type to check.
 * @see std::is_standard_layout
 */
template <typename T>
struct IsStandardLayout : BoolConstant<std::is_standard_layout<T>::value> {};
/// @brief Value form of IsStandardLayout.
template <typename T>
inline constexpr auto IsStandardLayout_V = IsStandardLayout<T>::value;

/**
 * @brief Checks whether T is an empty class type.
 * @tparam T Type to check.
 * @see std::is_empty
 */
template <typename T>
struct IsEmpty : BoolConstant<std::is_empty<T>::value> {};
/// @brief Value form of IsEmpty.
template <typename T>
inline constexpr auto IsEmpty_V = IsEmpty<T>::value;

/**
 * @brief Checks whether T is a polymorphic class type.
 * @tparam T Type to check.
 * @see std::is_polymorphic
 */
template <typename T>
struct IsPolymorphic : BoolConstant<std::is_polymorphic<T>::value> {};
/// @brief Value form of IsPolymorphic.
template <typename T>
inline constexpr auto IsPolymorphic_V = IsPolymorphic<T>::value;

/**
 * @brief Checks whether T is an abstract class type.
 * @tparam T Type to check.
 * @see std::is_abstract
 */
template <typename T>
struct IsAbstract : BoolConstant<std::is_abstract<T>::value> {};
/// @brief Value form of IsAbstract.
template <typename T>
inline constexpr auto IsAbstract_V = IsAbstract<T>::value;

/**
 * @brief Checks whether T is a final class or final overrider.
 * @tparam T Type to check.
 * @see std::is_final
 */
template <typename T>
struct IsFinal : BoolConstant<std::is_final<T>::value> {};
/// @brief Value form of IsFinal.
template <typename T>
inline constexpr auto IsFinal_V = IsFinal<T>::value;

/**
 * @brief Checks whether T is an aggregate type.
 * @tparam T Type to check.
 * @see std::is_aggregate
 */
template <typename T>
struct IsAggregate : BoolConstant<std::is_aggregate<T>::value> {};
/// @brief Value form of IsAggregate.
template <typename T>
inline constexpr auto IsAggregate_V = IsAggregate<T>::value;

/**
 * @brief Checks whether T is an unsigned arithmetic type.
 * @tparam T Type to check.
 * @see std::is_unsigned
 */
template <typename T>
struct IsUnsigned : BoolConstant<std::is_unsigned<T>::value> {};
/// @brief Value form of IsUnsigned.
template <typename T>
inline constexpr auto IsUnsigned_V = IsUnsigned<T>::value;

/**
 * @brief Checks whether T is a signed arithmetic type.
 * @tparam T Type to check.
 * @see std::is_signed
 */
template <typename T>
struct IsSigned : BoolConstant<std::is_signed<T>::value> {};
/// @brief Value form of IsSigned.
template <typename T>
inline constexpr auto IsSigned_V = IsSigned<T>::value;

/**
 * @brief Checks whether T is an array of known bound (e.g. `int[10]`).
 * @tparam T Type to check.
 * @see std::is_bounded_array
 *
 * ```cpp
 * static_assert(axio::IsBoundedArray<int[10]>::value);
 * static_assert(!axio::IsBoundedArray<int[]>::value);
 * ```
 */
template <typename T>
struct IsBoundedArray : FalseType {};
template <typename T, SizeT N>
struct IsBoundedArray<T[N]> : TrueType {};
/// @brief Value form of IsBoundedArray.
template <typename T>
inline constexpr auto IsBoundedArray_V = IsBoundedArray<T>::value;

/**
 * @brief Checks whether T is an array of unknown bound (e.g. `int[]`).
 * @tparam T Type to check.
 * @see std::is_unbounded_array
 *
 * ```cpp
 * static_assert(axio::IsUnboundedArray<int[]>::value);
 * static_assert(!axio::IsUnboundedArray<int[10]>::value);
 * ```
 */
template <typename T>
struct IsUnboundedArray : FalseType {};
template <typename T>
struct IsUnboundedArray<T[]> : TrueType {};
/// @brief Value form of IsUnboundedArray.
template <typename T>
inline constexpr auto IsUnboundedArray_V = IsUnboundedArray<T>::value;

/**
 * @brief Checks whether T is constructible from Args.
 * @tparam T Type to construct.
 * @tparam Args Constructor argument types.
 * @see std::is_constructible
 */
template <typename T, typename... Args>
struct IsConstructible
    : BoolConstant<std::is_constructible<T, Args...>::value> {};
/**
 * @brief Checks whether T is trivially constructible from Args.
 * @tparam T Type to construct.
 * @tparam Args Constructor argument types.
 * @see std::is_trivially_constructible
 */
template <typename T, typename... Args>
struct IsTriviallyConstructible
    : BoolConstant<std::is_trivially_constructible<T, Args...>::value> {};
/**
 * @brief Checks whether T is nothrow constructible from Args.
 * @tparam T Type to construct.
 * @tparam Args Constructor argument types.
 * @see std::is_nothrow_constructible
 */
template <typename T, typename... Args>
struct IsNothrowConstructible
    : BoolConstant<std::is_nothrow_constructible<T, Args...>::value> {};
/// @brief Value form of IsConstructible.
template <typename T, typename... Args>
inline constexpr auto IsConstructible_V = IsConstructible<T, Args...>::value;
/// @brief Value form of IsTriviallyConstructible.
template <typename T, typename... Args>
inline constexpr auto IsTriviallyConstructible_V =
    IsTriviallyConstructible<T, Args...>::value;
/// @brief Value form of IsNothrowConstructible.
template <typename T, typename... Args>
inline constexpr auto IsNothrowConstructible_V =
    IsNothrowConstructible<T, Args...>::value;

/**
 * @brief Checks whether T is default-constructible.
 * @tparam T Type to check.
 * @see std::is_default_constructible
 */
template <typename T>
struct IsDefaultConstructible
    : BoolConstant<std::is_default_constructible<T>::value> {};
/**
 * @brief Checks whether T is trivially default-constructible.
 * @tparam T Type to check.
 * @see std::is_trivially_default_constructible
 */
template <typename T>
struct IsTriviallyDefaultConstructible
    : BoolConstant<std::is_trivially_default_constructible<T>::value> {};
/**
 * @brief Checks whether T is nothrow default-constructible.
 * @tparam T Type to check.
 * @see std::is_nothrow_default_constructible
 */
template <typename T>
struct IsNothrowDefaultConstructible
    : BoolConstant<std::is_nothrow_default_constructible<T>::value> {};
/// @brief Value form of IsDefaultConstructible.
template <typename T>
inline constexpr auto IsDefaultConstructible_V =
    IsDefaultConstructible<T>::value;
/// @brief Value form of IsTriviallyDefaultConstructible.
template <typename T>
inline constexpr auto IsTriviallyDefaultConstructible_V =
    IsTriviallyDefaultConstructible<T>::value;
/// @brief Value form of IsNothrowDefaultConstructible.
template <typename T>
inline constexpr auto IsNothrowDefaultConstructible_V =
    IsNothrowDefaultConstructible<T>::value;

/**
 * @brief Checks whether T is copy-constructible.
 * @tparam T Type to check.
 * @see std::is_copy_constructible
 */
template <typename T>
struct IsCopyConstructible
    : BoolConstant<std::is_copy_constructible<T>::value> {};
/**
 * @brief Checks whether T is trivially copy-constructible.
 * @tparam T Type to check.
 * @see std::is_trivially_copy_constructible
 */
template <typename T>
struct IsTriviallyCopyConstructible
    : BoolConstant<std::is_trivially_copy_constructible<T>::value> {};
/**
 * @brief Checks whether T is nothrow copy-constructible.
 * @tparam T Type to check.
 * @see std::is_nothrow_copy_constructible
 */
template <typename T>
struct IsNothrowCopyConstructible
    : BoolConstant<std::is_nothrow_copy_constructible<T>::value> {};
/// @brief Value form of IsCopyConstructible.
template <typename T>
inline constexpr auto IsCopyConstructible_V = IsCopyConstructible<T>::value;
/// @brief Value form of IsTriviallyCopyConstructible.
template <typename T>
inline constexpr auto IsTriviallyCopyConstructible_V =
    IsTriviallyCopyConstructible<T>::value;
/// @brief Value form of IsNothrowCopyConstructible.
template <typename T>
inline constexpr auto IsNothrowCopyConstructible_V =
    IsNothrowCopyConstructible<T>::value;

/**
 * @brief Checks whether T is move-constructible.
 * @tparam T Type to check.
 * @see std::is_move_constructible
 */
template <typename T>
struct IsMoveConstructible
    : BoolConstant<std::is_move_constructible<T>::value> {};
/**
 * @brief Checks whether T is trivially move-constructible.
 * @tparam T Type to check.
 * @see std::is_trivially_move_constructible
 */
template <typename T>
struct IsTriviallyMoveConstructible
    : BoolConstant<std::is_trivially_move_constructible<T>::value> {};
/**
 * @brief Checks whether T is nothrow move-constructible.
 * @tparam T Type to check.
 * @see std::is_nothrow_move_constructible
 */
template <typename T>
struct IsNothrowMoveConstructible
    : BoolConstant<std::is_nothrow_move_constructible<T>::value> {};
/// @brief Value form of IsMoveConstructible.
template <typename T>
inline constexpr auto IsMoveConstructible_V = IsMoveConstructible<T>::value;
/// @brief Value form of IsTriviallyMoveConstructible.
template <typename T>
inline constexpr auto IsTriviallyMoveConstructible_V =
    IsTriviallyMoveConstructible<T>::value;
/// @brief Value form of IsNothrowMoveConstructible.
template <typename T>
inline constexpr auto IsNothrowMoveConstructible_V =
    IsNothrowMoveConstructible<T>::value;

/**
 * @brief Checks whether an object of type U can be assigned to T.
 * @tparam T Assignee type.
 * @tparam U Assigned-from type.
 * @see std::is_assignable
 */
template <typename T, typename U>
struct IsAssignable : BoolConstant<std::is_assignable<T, U>::value> {};
/**
 * @brief Checks whether assignment of U to T is trivial.
 * @tparam T Assignee type.
 * @tparam U Assigned-from type.
 * @see std::is_trivially_assignable
 */
template <typename T, typename U>
struct IsTriviallyAssignable
    : BoolConstant<std::is_trivially_assignable<T, U>::value> {};
/**
 * @brief Checks whether assignment of U to T cannot throw.
 * @tparam T Assignee type.
 * @tparam U Assigned-from type.
 * @see std::is_nothrow_assignable
 */
template <typename T, typename U>
struct IsNothrowAssignable
    : BoolConstant<std::is_nothrow_assignable<T, U>::value> {};
/// @brief Value form of IsAssignable.
template <typename T, typename U>
inline constexpr auto IsAssignable_V = IsAssignable<T, U>::value;
/// @brief Value form of IsTriviallyAssignable.
template <typename T, typename U>
inline constexpr auto IsTriviallyAssignable_V =
    IsTriviallyAssignable<T, U>::value;
/// @brief Value form of IsNothrowAssignable.
template <typename T, typename U>
inline constexpr auto IsNothrowAssignable_V = IsNothrowAssignable<T, U>::value;

/**
 * @brief Checks whether T is copy-assignable.
 * @tparam T Type to check.
 * @see std::is_copy_assignable
 */
template <typename T>
struct IsCopyAssignable : BoolConstant<std::is_copy_assignable<T>::value> {};
/**
 * @brief Checks whether T is trivially copy-assignable.
 * @tparam T Type to check.
 * @see std::is_trivially_copy_assignable
 */
template <typename T>
struct IsTriviallyCopyAssignable
    : BoolConstant<std::is_trivially_copy_assignable<T>::value> {};
/**
 * @brief Checks whether T is nothrow copy-assignable.
 * @tparam T Type to check.
 * @see std::is_nothrow_copy_assignable
 */
template <typename T>
struct IsNothrowCopyAssignable
    : BoolConstant<std::is_nothrow_copy_assignable<T>::value> {};
/// @brief Value form of IsCopyAssignable.
template <typename T>
inline constexpr auto IsCopyAssignable_V = IsCopyAssignable<T>::value;
/// @brief Value form of IsTriviallyCopyAssignable.
template <typename T>
inline constexpr auto IsTriviallyCopyAssignable_V =
    IsTriviallyCopyAssignable<T>::value;
/// @brief Value form of IsNothrowCopyAssignable.
template <typename T>
inline constexpr auto IsNothrowCopyAssignable_V =
    IsNothrowCopyAssignable<T>::value;

/**
 * @brief Checks whether T is move-assignable.
 * @tparam T Type to check.
 * @see std::is_move_assignable
 */
template <typename T>
struct IsMoveAssignable : BoolConstant<std::is_move_assignable<T>::value> {};
/**
 * @brief Checks whether T is trivially move-assignable.
 * @tparam T Type to check.
 * @see std::is_trivially_move_assignable
 */
template <typename T>
struct IsTriviallyMoveAssignable
    : BoolConstant<std::is_trivially_move_assignable<T>::value> {};
/**
 * @brief Checks whether T is nothrow move-assignable.
 * @tparam T Type to check.
 * @see std::is_nothrow_move_assignable
 */
template <typename T>
struct IsNothrowMoveAssignable
    : BoolConstant<std::is_nothrow_move_assignable<T>::value> {};
/// @brief Value form of IsMoveAssignable.
template <typename T>
inline constexpr auto IsMoveAssignable_V = IsMoveAssignable<T>::value;
/// @brief Value form of IsTriviallyMoveAssignable.
template <typename T>
inline constexpr auto IsTriviallyMoveAssignable_V =
    IsTriviallyMoveAssignable<T>::value;
/// @brief Value form of IsNothrowMoveAssignable.
template <typename T>
inline constexpr auto IsNothrowMoveAssignable_V =
    IsNothrowMoveAssignable<T>::value;

/**
 * @brief Checks whether T is destructible.
 * @tparam T Type to check.
 * @see std::is_destructible
 */
template <typename T>
struct IsDestructible : BoolConstant<std::is_destructible<T>::value> {};
/**
 * @brief Checks whether T is trivially destructible.
 * @tparam T Type to check.
 * @see std::is_trivially_destructible
 */
template <typename T>
struct IsTriviallyDestructible
    : BoolConstant<std::is_trivially_destructible<T>::value> {};
/**
 * @brief Checks whether T's destructor cannot throw.
 * @tparam T Type to check.
 * @see std::is_nothrow_destructible
 */
template <typename T>
struct IsNothrowDestructible
    : BoolConstant<std::is_nothrow_destructible<T>::value> {};
/// @brief Value form of IsDestructible.
template <typename T>
inline constexpr auto IsDestructible_V = IsDestructible<T>::value;
/// @brief Value form of IsTriviallyDestructible.
template <typename T>
inline constexpr auto IsTriviallyDestructible_V =
    IsTriviallyDestructible<T>::value;
/// @brief Value form of IsNothrowDestructible.
template <typename T>
inline constexpr auto IsNothrowDestructible_V = IsNothrowDestructible<T>::value;

/**
 * @brief Computes the alignment requirement of T, in bytes.
 * @tparam T Type to check.
 * @see std::alignment_of
 */
template <typename T>
struct AlignmentOf : IntegralConstant<SizeT, std::alignment_of<T>::value> {};
/// @brief Value form of AlignmentOf.
template <typename T>
inline constexpr auto AlignmentOf_V = AlignmentOf<T>::value;

/**
 * @brief Computes the number of array dimensions of T.
 * @tparam T Type to check.
 * @see std::rank
 */
template <typename T>
struct Rank : IntegralConstant<SizeT, std::rank<T>::value> {};
/// @brief Value form of Rank.
template <typename T>
inline constexpr auto Rank_V = Rank<T>::value;

/**
 * @brief Computes the bound of array dimension N of T (0 if unbounded).
 * @tparam T Array type to check.
 * @tparam N Zero-based dimension index.
 * @see std::extent
 */
template <typename T, unsigned N = 0>
struct Extent : IntegralConstant<SizeT, std::extent<T, N>::value> {};
/// @brief Value form of Extent.
template <typename T, unsigned N = 0>
inline constexpr auto Extent_V = Extent<T, N>::value;

/**
 * @brief Checks whether T and U are the same type.
 * @tparam T First type.
 * @tparam U Second type.
 * @see std::is_same
 */
template <typename T, typename U>
struct IsSame : BoolConstant<std::is_same<T, U>::value> {};
/// @brief Value form of IsSame.
template <typename T, typename U>
inline constexpr auto IsSame_V = IsSame<T, U>::value;

/**
 * @brief Checks whether Base is a base class of Derived (or the same class).
 * @tparam Base Candidate base class.
 * @tparam Derived Candidate derived class.
 * @see std::is_base_of
 */
template <typename Base, typename Derived>
struct IsBaseOf : BoolConstant<std::is_base_of<Base, Derived>::value> {};
/// @brief Value form of IsBaseOf.
template <typename Base, typename Derived>
inline constexpr auto IsBaseOf_V = IsBaseOf<Base, Derived>::value;

/**
 * @brief Checks whether an expression of type From can be implicitly
 * converted to To.
 * @tparam From Source type.
 * @tparam To Target type.
 * @see std::is_convertible
 */
template <typename From, typename To>
struct IsConvertible : BoolConstant<std::is_convertible<From, To>::value> {};
/// @brief Value form of IsConvertible.
template <typename From, typename To>
inline constexpr auto IsConvertible_V = IsConvertible<From, To>::value;

/**
 * @brief Checks whether Fn can be invoked with Args.
 * @tparam Fn Callable type.
 * @tparam Args Argument types.
 * @see std::is_invocable
 */
template <typename Fn, typename... Args>
struct IsInvocable : BoolConstant<std::is_invocable<Fn, Args...>::value> {};
/// @brief Value form of IsInvocable.
template <typename Fn, typename... Args>
inline constexpr auto IsInvocable_V = IsInvocable<Fn, Args...>::value;

/**
 * @brief Checks whether Fn can be invoked with Args without throwing.
 * @tparam Fn Callable type.
 * @tparam Args Argument types.
 * @see std::is_nothrow_invocable
 */
template <typename Fn, typename... Args>
struct IsNothrowInvocable
    : BoolConstant<std::is_nothrow_invocable<Fn, Args...>::value> {};
/// @brief Value form of IsNothrowInvocable.
template <typename Fn, typename... Args>
inline constexpr auto IsNothrowInvocable_V =
    IsNothrowInvocable<Fn, Args...>::value;

/**
 * @brief Checks whether Fn is invocable with Args and the result is
 * convertible to R.
 * @tparam R Expected (or compatible) result type.
 * @tparam Fn Callable type.
 * @tparam Args Argument types.
 * @see std::is_invocable_r
 */
template <typename R, typename Fn, typename... Args>
struct IsInvocableR : BoolConstant<std::is_invocable_r<R, Fn, Args...>::value> {
};
/// @brief Value form of IsInvocableR.
template <typename Fn, typename... Args>
inline constexpr auto IsInvocableR_V = IsInvocableR<Fn, Args...>::value;

/**
 * @brief Checks whether Fn is nothrow invocable with Args and the result is
 * convertible to R.
 * @tparam R Expected (or compatible) result type.
 * @tparam Fn Callable type.
 * @tparam Args Argument types.
 * @see std::is_nothrow_invocable_r
 */
template <typename R, typename Fn, typename... Args>
struct IsNothrowInvocableR
    : BoolConstant<std::is_nothrow_invocable_r<R, Fn, Args...>::value> {};
/// @brief Value form of IsNothrowInvocableR.
template <typename Fn, typename... Args>
inline constexpr auto IsNothrowInvocableR_V =
    IsNothrowInvocableR<Fn, Args...>::value;

/**
 * @brief Removes the top-level const qualifier from T.
 * @tparam T Type to strip.
 * @see std::remove_const
 */
template <typename T>
struct RemoveConst {
  using type = typename std::remove_const<T>::type;
};
/// @brief Alias form of RemoveConst.
template <typename T>
using RemoveConst_T = typename RemoveConst<T>::type;

/**
 * @brief Removes the top-level volatile qualifier from T.
 * @tparam T Type to strip.
 * @see std::remove_volatile
 */
template <typename T>
struct RemoveVolatile {
  using type = typename std::remove_volatile<T>::type;
};
/// @brief Alias form of RemoveVolatile.
template <typename T>
using RemoveVolatile_T = typename RemoveVolatile<T>::type;

/**
 * @brief Removes top-level const and volatile qualifiers from T.
 * @tparam T Type to strip.
 * @see std::remove_cv
 */
template <typename T>
struct RemoveCV {
  using type = typename std::remove_cv<T>::type;
};
/// @brief Alias form of RemoveCV.
template <typename T>
using RemoveCV_T = typename RemoveCV<T>::type;

/**
 * @brief Adds a top-level const qualifier to T.
 * @tparam T Type to qualify.
 * @see std::add_const
 */
template <typename T>
struct AddConst {
  using type = typename std::add_const<T>::type;
};
/// @brief Alias form of AddConst.
template <typename T>
using AddConst_T = typename AddConst<T>::type;

/**
 * @brief Adds a top-level volatile qualifier to T.
 * @tparam T Type to qualify.
 * @see std::add_volatile
 */
template <typename T>
struct AddVolatile {
  using type = typename std::add_volatile<T>::type;
};
/// @brief Alias form of AddVolatile.
template <typename T>
using AddVolatile_T = typename AddVolatile<T>::type;

/**
 * @brief Adds top-level const and volatile qualifiers to T.
 * @tparam T Type to qualify.
 * @see std::add_cv
 */
template <typename T>
struct AddCV {
  using type = typename std::add_cv<T>::type;
};
/// @brief Alias form of AddCV.
template <typename T>
using AddCV_T = typename AddCV<T>::type;

/**
 * @brief Removes the reference qualifier from T, if any.
 * @tparam T Type to strip.
 * @see std::remove_reference
 */
template <typename T>
struct RemoveReference {
  using type = typename std::remove_reference<T>::type;
};
/// @brief Alias form of RemoveReference.
template <typename T>
using RemoveReference_T = typename RemoveReference<T>::type;

/**
 * @brief Forms an lvalue reference to T, following reference-collapsing
 * rules.
 * @tparam T Referenced type.
 * @see std::add_lvalue_reference
 */
template <typename T>
struct AddLValueReference {
  using type = typename std::add_lvalue_reference<T>::type;
};
/// @brief Alias form of AddLValueReference.
template <typename T>
using AddLValueReference_T = typename AddLValueReference<T>::type;

/**
 * @brief Forms an rvalue reference to T, following reference-collapsing
 * rules.
 * @tparam T Referenced type.
 * @see std::add_rvalue_reference
 */
template <typename T>
struct AddRValueReference {
  using type = typename std::add_rvalue_reference<T>::type;
};
/// @brief Alias form of AddRValueReference.
template <typename T>
using AddRValueReference_T = typename AddRValueReference<T>::type;

/**
 * @brief Removes one level of pointer from T, if any.
 * @tparam T Type to strip.
 * @see std::remove_pointer
 */
template <typename T>
struct RemovePointer {
  using type = typename std::remove_pointer<T>::type;
};
/// @brief Alias form of RemovePointer.
template <typename T>
using RemovePointer_T = typename RemovePointer<T>::type;

/**
 * @brief Forms a pointer to T.
 * @tparam T Pointed-to type.
 * @see std::add_pointer
 */
template <typename T>
struct AddPointer {
  using type = typename std::add_pointer<T>::type;
};
/// @brief Alias form of AddPointer.
template <typename T>
using AddPointer_T = typename AddPointer<T>::type;

/**
 * @brief Produces the signed integer type corresponding to T.
 * @tparam T Integral or enum type to convert.
 * @see std::make_signed
 */
template <typename T>
struct MakeSigned {
  using type = typename std::make_signed<T>::type;
};
/// @brief Alias form of MakeSigned.
template <typename T>
using MakeSigned_T = typename MakeSigned<T>::type;

/**
 * @brief Produces the unsigned integer type corresponding to T.
 * @tparam T Integral or enum type to convert.
 * @see std::make_unsigned
 */
template <typename T>
struct MakeUnsigned {
  using type = typename std::make_unsigned<T>::type;
};
/// @brief Alias form of MakeUnsigned.
template <typename T>
using MakeUnsigned_T = typename MakeUnsigned<T>::type;

/**
 * @brief Removes one dimension from array type T.
 * @tparam T Array type to strip.
 * @see std::remove_extent
 */
template <typename T>
struct RemoveExtent {
  using type = typename std::remove_extent<T>::type;
};
/// @brief Alias form of RemoveExtent.
template <typename T>
using RemoveExtent_T = typename RemoveExtent<T>::type;

/**
 * @brief Removes all dimensions from array type T.
 * @tparam T Array type to strip.
 * @see std::remove_all_extents
 */
template <typename T>
struct RemoveAllExtents {
  using type = typename std::remove_all_extents<T>::type;
};
/// @brief Alias form of RemoveAllExtents.
template <typename T>
using RemoveAllExtents_T = typename RemoveAllExtents<T>::type;

/**
 * @brief Applies the type transformations performed during pass-by-value
 * argument passing (array/function decay, cv/reference removal).
 * @tparam T Type to decay.
 * @see std::decay
 */
template <typename T>
struct Decay {
  using type = typename std::decay<T>::type;
};
/// @brief Alias form of Decay.
template <typename T>
using Decay_T = typename Decay<T>::type;

/**
 * @brief Removes both cv-qualifiers and references from a type.
 *
 * @tparam T Type to strip.
 * @see std::remove_cvref
 *
 * Example:
 * ```cpp
 * using Type = axio::RemoveCVRef_T<const volatile int&>;
 * static_assert(axio::IsSame_V<Type, int>);
 * ```
 */
template <typename T>
struct RemoveCVRef {
  using type =
      typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};
/// @brief Alias form of RemoveCVRef.
template <typename T>
using RemoveCVRef_T = typename RemoveCVRef<T>::type;

/**
 * @brief Conditionally provides a member typedef when B is true.
 *
 * @tparam B Condition controlling whether `type` is defined.
 * @tparam T Type to expose as `type` when @p B is true.
 * @see std::enable_if
 *
 * Example:
 * ```cpp
 * template <typename T,
 *           typename = axio::EnableIf_T<axio::IsIntegral<T>>>
 * void Foo(T value);
 * ```
 */
template <Bool B, typename T = void>
struct EnableIf {};
template <typename T>
struct EnableIf<true, T> {
  using type = T;
};
/// @brief Alias form of EnableIf.
template <Bool B, typename T = void>
using EnableIf_T = typename EnableIf<B, T>::type;

/**
 * @brief Selects between T and F based on a compile-time condition.
 * @tparam B Condition controlling the selection.
 * @tparam T Type selected when @p B is true.
 * @tparam F Type selected when @p B is false.
 * @see std::conditional
 */
template <Bool B, typename T, typename F>
struct Conditional {
  using type = typename std::conditional<B, T, F>::type;
};
/// @brief Alias form of Conditional.
template <Bool B, typename T, typename F>
using Conditional_T = typename Conditional<B, T, F>::type;

/**
 * @brief Determines the common type to which all of Types can be converted.
 * @tparam Types Candidate types.
 * @see std::common_type
 */
template <typename... Types>
struct CommonType {
  using type = typename std::common_type<Types...>::type;
};
/// @brief Alias form of CommonType.
template <typename... Types>
using CommonType_T = typename CommonType<Types...>::type;

/**
 * @brief Retrieves the underlying integral type of an enumeration.
 * @tparam T Enumeration type.
 * @see std::underlying_type
 */
template <typename T>
struct UnderlyingType {
  using type = typename std::underlying_type<T>::type;
};
/// @brief Alias form of UnderlyingType.
template <typename T>
using UnderlyingType_T = typename UnderlyingType<T>::type;

/**
 * @brief Deduces the result type of invoking Fn with Args.
 * @tparam Fn Callable type.
 * @tparam Args Argument types.
 * @see std::invoke_result
 */
template <typename Fn, typename... Args>
struct InvokeResult {
  using type = typename std::invoke_result<Fn, Args...>::type;
};
/// @brief Alias form of InvokeResult.
template <typename Fn, typename... Args>
using InvokeResult_T = typename InvokeResult<Fn, Args...>::type;

/**
 * @brief Maps any list of types to void.
 *
 * Commonly used as a SFINAE detection helper.
 *
 * @see std::void_t
 *
 * Example:
 * ```cpp
 * template <typename T, typename = Void_T<>>
 * struct HasValueType : FalseType {};
 *
 * template <typename T>
 * struct HasValueType<T, Void_T<typename T::ValueType>>
 *     : TrueType {};
 * ```
 */
template <typename...>
using Void_T = void;

/**
 * @brief Preserves a type without modification.
 *
 * Useful for inhibiting template argument deduction.
 *
 * @tparam T Type to preserve.
 * @see std::type_identity
 *
 * Example:
 * ```cpp
 * using Type = TypeIdentity_T<const int>;
 * static_assert(std::is_same_v<Type, const int>);
 * ```
 */
template <typename T>
struct TypeIdentity {
  using type = T;
};
/// @brief Alias form of TypeIdentity.
template <typename T>
using TypeIdentity_T = typename TypeIdentity<T>::type;

/**
 * @brief Performs a logical AND of Traits' `::value` members, short-circuiting.
 * @tparam Traits Trait types exposing a static boolean `value`.
 * @see std::conjunction
 */
template <typename... Traits>
struct Conjunction : BoolConstant<std::conjunction<Traits...>::value> {};
/// @brief Value form of Conjunction.
template <typename... Traits>
inline constexpr auto Conjunction_V = Conjunction<Traits...>::value;

/**
 * @brief Performs a logical OR of Traits' `::value` members, short-circuiting.
 * @tparam Traits Trait types exposing a static boolean `value`.
 * @see std::disjunction
 */
template <typename... Traits>
struct Disjunction : BoolConstant<std::disjunction<Traits...>::value> {};
/// @brief Value form of Disjunction.
template <typename... Traits>
inline constexpr auto Disjunction_V = Disjunction<Traits...>::value;

/**
 * @brief Negates the `::value` member of Trait.
 * @tparam Trait Trait type exposing a static boolean `value`.
 * @see std::negation
 */
template <typename Trait>
struct Negation : BoolConstant<std::negation<Trait>::value> {};
/// @brief Value form of Negation.
template <typename... Traits>
inline constexpr auto Negation_V = Negation<Traits...>::value;

/**
 * @brief Checks if T is exactly one of Types.
 *
 * Evaluates to true if T matches any type in the parameter pack.
 *
 * @tparam T Type to search for.
 * @tparam Types Candidate types.
 */
template <typename T, typename... Types>
struct IsAnyOf : Disjunction<IsSame<T, Types>...> {};
/// @brief Value form of IsAnyOf.
template <typename T, typename... Types>
inline constexpr auto IsAnyOf_V = IsAnyOf<T, Types...>::value;

/**
 * @brief Checks if T is none of Types.
 * @tparam T Type to search for.
 * @tparam Types Candidate types.
 */
template <typename T, typename... Types>
struct IsNoneOf : BoolConstant<!IsAnyOf<T, Types...>::value> {};
/// @brief Value form of IsNoneOf.
template <typename T, typename... Types>
inline constexpr auto IsNoneOf_V = IsNoneOf<T, Types...>::value;

/**
 * @brief Detects whether a type is a specialization of a given template.
 *
 * @tparam T Type to check.
 * @tparam Template Class template to match against.
 *
 * Example:
 * ```cpp
 * IsSpecializationOf<std::vector<int>, std::vector>::value == true
 * ```
 */
template <typename T, template <typename...> typename Template>
struct IsSpecializationOf : FalseType {};
template <template <typename...> typename Template, typename... Args>
struct IsSpecializationOf<Template<Args...>, Template> : TrueType {};
/// @brief Value form of IsSpecializationOf.
template <typename T, template <typename...> typename Template>
inline constexpr auto IsSpecializationOf_V =
    IsSpecializationOf<T, Template>::value;

/**
 * @brief Checks whether type T is complete at the point of instantiation.
 *
 * Uses `sizeof(T)` to trigger SFINAE. If T is incomplete, evaluation fails
 * and IsComplete resolves to FalseType.
 *
 * @tparam T Type to check.
 */
template <typename T, typename = void>
struct IsComplete : FalseType {};
template <typename T>
struct IsComplete<T, Void_T<decltype(sizeof(T))>> : TrueType {};
/// @brief Value form of IsComplete.
template <typename T>
inline constexpr auto IsComplete_V = IsComplete<T>::value;

namespace internal {
template <template <typename...> typename Op, typename, typename... Args>
struct IsDetectedImpl : FalseType {};
template <template <typename...> typename Op, typename... Args>
struct IsDetectedImpl<Op, Void_T<Op<Args...>>, Args...> : TrueType {};
}  // namespace internal

/**
 * @brief Detects whether an expression Op<Args...> is valid (well-formed).
 *
 * This is commonly used to:
 * - Detect presence of nested types (e.g., `T::type`)
 * - Detect valid expressions (e.g., `T + U`, `T.begin()`, etc.)
 * - Enable conditional template specialization based on capabilities
 *
 * @tparam Op Template alias representing the expression to test.
 * @tparam Args Arguments substituted into @p Op.
 *
 * Example:
 * ```cpp
 * template <typename T>
 * using HasSize = decltype(std::declval<T>().size());
 *
 * static_assert(IsDetected_V<HasSize, std::vector<int>> == true);
 * static_assert(IsDetected_V<HasSize, int> == false);
 * ```
 */
template <template <typename...> typename Op, typename... Args>
struct IsDetected : internal::IsDetectedImpl<Op, void, Args...> {};
/// @brief Value form of IsDetected.
template <template <typename...> typename Op, typename... Args>
inline constexpr auto IsDetected_V = IsDetected<Op, Args...>::value;

/**
 * @brief Simple variadic type container.
 * @tparam Types Types held by the list.
 */
template <typename... Types>
struct TypeList {};

/**
 * @brief Checks if Lhs and Rhs are equality comparable (`==`) with a result
 * convertible to bool.
 * @tparam Lhs Left-hand operand type.
 * @tparam Rhs Right-hand operand type (defaults to Lhs).
 */
template <typename Lhs, typename Rhs = Lhs, typename = void>
struct IsEqualityComparable : FalseType {};
template <typename Lhs, typename Rhs>
struct IsEqualityComparable<
    Lhs,
    Rhs,
    Void_T<decltype(std::declval<Lhs>() == std::declval<Rhs>())>>
    : BoolConstant<
          IsConvertible<decltype(std::declval<Lhs>() == std::declval<Rhs>()),
                        bool>::value> {};
/// @brief Value form of IsEqualityComparable.
template <typename Lhs, typename Rhs = Lhs>
inline constexpr auto IsEqualityComparable_V =
    IsEqualityComparable<Lhs, Rhs>::value;

/**
 * @brief Checks if `Lhs < Rhs` is valid and convertible to bool.
 * @tparam Lhs Left-hand operand type.
 * @tparam Rhs Right-hand operand type (defaults to Lhs).
 */
template <typename Lhs, typename Rhs = Lhs, typename = void>
struct IsLessThanComparable : FalseType {};
template <typename Lhs, typename Rhs>
struct IsLessThanComparable<
    Lhs,
    Rhs,
    Void_T<decltype(std::declval<Lhs>() < std::declval<Rhs>())>>
    : BoolConstant<
          IsConvertible<decltype(std::declval<Lhs>() < std::declval<Rhs>()),
                        bool>::value> {};
/// @brief Value form of IsLessThanComparable.
template <typename Lhs, typename Rhs = Lhs>
inline constexpr auto IsLessThanComparable_V =
    IsLessThanComparable<Lhs, Rhs>::value;

/**
 * @brief Checks if `Lhs > Rhs` is valid and convertible to bool.
 * @tparam Lhs Left-hand operand type.
 * @tparam Rhs Right-hand operand type (defaults to Lhs).
 */
template <typename Lhs, typename Rhs = Lhs, typename = void>
struct IsGreaterThanComparable : FalseType {};
template <typename Lhs, typename Rhs>
struct IsGreaterThanComparable<
    Lhs,
    Rhs,
    Void_T<decltype(std::declval<Lhs>() > std::declval<Rhs>())>>
    : BoolConstant<
          IsConvertible<decltype(std::declval<Lhs>() > std::declval<Rhs>()),
                        bool>::value> {};
/// @brief Value form of IsGreaterThanComparable.
template <typename Lhs, typename Rhs = Lhs>
inline constexpr auto IsGreaterThanComparable_V =
    IsGreaterThanComparable<Lhs, Rhs>::value;

/**
 * @brief Determines whether a type should use Empty Base Optimization (EBO).
 *
 * True when T is empty, a class type, and not marked final:
 * `IsEmpty<T>::value && IsClass<T>::value && !IsFinal<T>::value`.
 *
 * @tparam T Type to check.
 */
template <typename T>
struct ShouldUseEBO : BoolConstant<IsEmpty<T>::value && IsClass<T>::value &&
                                   !IsFinal<T>::value> {};
/// @brief Value form of ShouldUseEBO.
template <typename T>
inline constexpr auto ShouldUseEBO_V = ShouldUseEBO<T>::value;

/**
 * @brief Unwraps std::reference_wrapper<T> into T&; identity for other types.
 * @tparam T Type to unwrap.
 */
template <typename T>
struct UnwrapReferenceWrapper {
  using type = T;
};
template <typename T>
struct UnwrapReferenceWrapper<std::reference_wrapper<T>> {
  using type = T&;
};
/// @brief Alias form of UnwrapReferenceWrapper.
template <typename T>
using UnwrapReferenceWrapper_T = typename UnwrapReferenceWrapper<T>::type;

/**
 * @brief Checks whether T is a std::reference_wrapper specialization.
 * @tparam T Type to check.
 */
template <typename T>
struct IsReferenceWrapper : FalseType {};
template <typename T>
struct IsReferenceWrapper<std::reference_wrapper<T>> : TrueType {};
/// @brief Value form of IsReferenceWrapper.
template <typename T>
inline constexpr auto IsReferenceWrapper_V = IsReferenceWrapper<T>::value;
}  // namespace axio

/**
 * @brief Defines bitwise operators for enum types.
 *
 * Generates `~`, `|`, `&`, `^`, `|=`, `&=`, `^=` operators for @p Enum,
 * implemented via conversion to its underlying integral type.
 *
 * @param Enum Enumeration type to generate operators for.
 */
#define AXIO_DEFINE_ENUM_BITWISE(Enum)                                   \
  constexpr Enum operator~(Enum value) {                                 \
    using U = ::axio::UnderlyingType_T<Enum>;                            \
    return static_cast<Enum>(~static_cast<U>(value));                    \
  }                                                                      \
                                                                         \
  constexpr Enum operator|(Enum lhs, Enum rhs) {                         \
    using U = ::axio::UnderlyingType_T<Enum>;                            \
    return static_cast<Enum>(static_cast<U>(lhs) | static_cast<U>(rhs)); \
  }                                                                      \
                                                                         \
  constexpr Enum operator&(Enum lhs, Enum rhs) {                         \
    using U = ::axio::UnderlyingType_T<Enum>;                            \
    return static_cast<Enum>(static_cast<U>(lhs) & static_cast<U>(rhs)); \
  }                                                                      \
                                                                         \
  constexpr Enum operator^(Enum lhs, Enum rhs) {                         \
    using U = ::axio::UnderlyingType_T<Enum>;                            \
    return static_cast<Enum>(static_cast<U>(lhs) ^ static_cast<U>(rhs)); \
  }                                                                      \
                                                                         \
  constexpr Enum& operator|=(Enum& lhs, Enum rhs) {                      \
    lhs = lhs | rhs;                                                     \
    return lhs;                                                          \
  }                                                                      \
                                                                         \
  constexpr Enum& operator&=(Enum& lhs, Enum rhs) {                      \
    lhs = lhs & rhs;                                                     \
    return lhs;                                                          \
  }                                                                      \
                                                                         \
  constexpr Enum& operator^=(Enum& lhs, Enum rhs) {                      \
    lhs = lhs ^ rhs;                                                     \
    return lhs;                                                          \
  }
#endif