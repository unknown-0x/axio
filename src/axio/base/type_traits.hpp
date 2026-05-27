#ifndef AXIO_BASE_TYPE_TRAITS_HPP_
#define AXIO_BASE_TYPE_TRAITS_HPP_

#include "types.hpp"

#include <functional>
#include <type_traits>

namespace axio {
template <typename T, T VALUE>
struct IntegralConstant {
  static constexpr T value = VALUE;
  using ValueType = T;

  constexpr operator ValueType() const noexcept { return value; }
  constexpr ValueType operator()() const noexcept { return value; }
};

template <Bool VALUE>
using BoolConstant = IntegralConstant<Bool, VALUE>;
using TrueType = BoolConstant<true>;
using FalseType = BoolConstant<false>;

template <typename T>
struct IsVoid : BoolConstant<std::is_void<T>::value> {};
template <typename T>
inline constexpr auto IsVoid_V = IsVoid<T>::value;

template <typename T>
struct IsNullPointer : BoolConstant<std::is_null_pointer<T>::value> {};
template <typename T>
inline constexpr auto IsNullPointer_V = IsNullPointer<T>::value;

template <typename T>
struct IsIntegral : BoolConstant<std::is_integral<T>::value> {};
template <typename T>
inline constexpr auto IsIntegral_V = IsIntegral<T>::value;

template <typename T>
struct IsFloatingPoint : BoolConstant<std::is_floating_point<T>::value> {};
template <typename T>
inline constexpr auto IsFloatingPoint_V = IsFloatingPoint<T>::value;

template <typename T>
struct IsArray : BoolConstant<std::is_array<T>::value> {};
template <typename T>
inline constexpr auto IsArray_V = IsArray<T>::value;

template <typename T>
struct IsEnum : BoolConstant<std::is_enum<T>::value> {};
template <typename T>
inline constexpr auto IsEnum_V = IsEnum<T>::value;

template <typename T>
struct IsUnion : BoolConstant<std::is_union<T>::value> {};
template <typename T>
inline constexpr auto IsUnion_V = IsUnion<T>::value;

template <typename T>
struct IsClass : BoolConstant<std::is_class<T>::value> {};
template <typename T>
inline constexpr auto IsClass_V = IsClass<T>::value;

template <typename T>
struct IsFunction : BoolConstant<std::is_function<T>::value> {};
template <typename T>
inline constexpr auto IsFunction_V = IsFunction<T>::value;

template <typename T>
struct IsPointer : BoolConstant<std::is_pointer<T>::value> {};
template <typename T>
inline constexpr auto IsPointer_V = IsPointer<T>::value;

template <typename T>
struct IsLvalueReference : BoolConstant<std::is_lvalue_reference<T>::value> {};
template <typename T>
inline constexpr auto IsLvalueReference_V = IsLvalueReference<T>::value;

template <typename T>
struct IsRvalueReference : BoolConstant<std::is_rvalue_reference<T>::value> {};
template <typename T>
inline constexpr auto IsRvalueReference_V = IsRvalueReference<T>::value;

template <typename T>
struct IsMemberObjectPointer
    : BoolConstant<std::is_member_object_pointer<T>::value> {};
template <typename T>
inline constexpr auto IsMemberObjectPointer_V = IsMemberObjectPointer<T>::value;

template <typename T>
struct IsMemberFunctionPointer
    : BoolConstant<std::is_member_function_pointer<T>::value> {};
template <typename T>
inline constexpr auto IsMemberFunctionPointer_V =
    IsMemberFunctionPointer<T>::value;

template <typename T>
struct IsFundamental : BoolConstant<std::is_fundamental<T>::value> {};
template <typename T>
inline constexpr auto IsFundamental_V = IsFundamental<T>::value;

template <typename T>
struct IsArithmetic : BoolConstant<std::is_arithmetic<T>::value> {};
template <typename T>
inline constexpr auto IsArithmetic_V = IsArithmetic<T>::value;

template <typename T>
struct IsScalar : BoolConstant<std::is_scalar<T>::value> {};
template <typename T>
inline constexpr auto IsScalar_V = IsScalar<T>::value;

template <typename T>
struct IsObject : BoolConstant<std::is_object<T>::value> {};
template <typename T>
inline constexpr auto IsObject_V = IsObject<T>::value;

template <typename T>
struct IsCompound : BoolConstant<std::is_compound<T>::value> {};
template <typename T>
inline constexpr auto IsCompound_V = IsCompound<T>::value;

template <typename T>
struct IsReference : BoolConstant<std::is_reference<T>::value> {};
template <typename T>
inline constexpr auto IsReference_V = IsReference<T>::value;

template <typename T>
struct IsMemberPointer : BoolConstant<std::is_member_pointer<T>::value> {};
template <typename T>
inline constexpr auto IsMemberPointer_V = IsMemberPointer<T>::value;

template <typename T>
struct IsConst : BoolConstant<std::is_const<T>::value> {};
template <typename T>
inline constexpr auto IsConst_V = IsConst<T>::value;

template <typename T>
struct IsVolatile : BoolConstant<std::is_volatile<T>::value> {};
template <typename T>
inline constexpr auto IsVolatile_V = IsVolatile<T>::value;

template <typename T>
struct IsTriviallyCopyable
    : BoolConstant<std::is_trivially_copyable<T>::value> {};
template <typename T>
inline constexpr auto IsTriviallyCopyable_V = IsTriviallyCopyable<T>::value;

template <typename T>
struct IsStandardLayout : BoolConstant<std::is_standard_layout<T>::value> {};
template <typename T>
inline constexpr auto IsStandardLayout_V = IsStandardLayout<T>::value;

template <typename T>
struct IsEmpty : BoolConstant<std::is_empty<T>::value> {};
template <typename T>
inline constexpr auto IsEmpty_V = IsEmpty<T>::value;

template <typename T>
struct IsPolymorphic : BoolConstant<std::is_polymorphic<T>::value> {};
template <typename T>
inline constexpr auto IsPolymorphic_V = IsPolymorphic<T>::value;

template <typename T>
struct IsAbstract : BoolConstant<std::is_abstract<T>::value> {};
template <typename T>
inline constexpr auto IsAbstract_V = IsAbstract<T>::value;

template <typename T>
struct IsFinal : BoolConstant<std::is_final<T>::value> {};
template <typename T>
inline constexpr auto IsFinal_V = IsFinal<T>::value;

template <typename T>
struct IsAggregate : BoolConstant<std::is_aggregate<T>::value> {};
template <typename T>
inline constexpr auto IsAggregate_V = IsAggregate<T>::value;

template <typename T>
struct IsUnsigned : BoolConstant<std::is_unsigned<T>::value> {};
template <typename T>
inline constexpr auto IsUnsigned_V = IsUnsigned<T>::value;

template <typename T>
struct IsSigned : BoolConstant<std::is_signed<T>::value> {};
template <typename T>
inline constexpr auto IsSigned_V = IsSigned<T>::value;

template <typename T>
struct IsBoundedArray : FalseType {};
template <typename T, SizeT N>
struct IsBoundedArray<T[N]> : TrueType {};
template <typename T>
inline constexpr auto IsBoundedArray_V = IsBoundedArray<T>::value;

template <typename T>
struct IsUnboundedArray : FalseType {};
template <typename T>
struct IsUnboundedArray<T[]> : TrueType {};
template <typename T>
inline constexpr auto IsUnboundedArray_V = IsUnboundedArray<T>::value;

template <typename T, typename... Args>
struct IsConstructible
    : BoolConstant<std::is_constructible<T, Args...>::value> {};
template <typename T, typename... Args>
struct IsTriviallyConstructible
    : BoolConstant<std::is_trivially_constructible<T, Args...>::value> {};
template <typename T, typename... Args>
struct IsNothrowConstructible
    : BoolConstant<std::is_nothrow_constructible<T, Args...>::value> {};
template <typename T, typename... Args>
inline constexpr auto IsConstructible_V = IsConstructible<T, Args...>::value;
template <typename T, typename... Args>
inline constexpr auto IsTriviallyConstructible_V =
    IsTriviallyConstructible<T, Args...>::value;
template <typename T, typename... Args>
inline constexpr auto IsNothrowConstructible_V =
    IsNothrowConstructible<T, Args...>::value;

template <typename T>
struct IsDefaultConstructible
    : BoolConstant<std::is_default_constructible<T>::value> {};
template <typename T>
struct IsTriviallyDefaultConstructible
    : BoolConstant<std::is_trivially_default_constructible<T>::value> {};
template <typename T>
struct IsNothrowDefaultConstructible
    : BoolConstant<std::is_nothrow_default_constructible<T>::value> {};
template <typename T>
inline constexpr auto IsDefaultConstructible_V =
    IsDefaultConstructible<T>::value;
template <typename T>
inline constexpr auto IsTriviallyDefaultConstructible_V =
    IsTriviallyDefaultConstructible<T>::value;
template <typename T>
inline constexpr auto IsNothrowDefaultConstructible_V =
    IsNothrowDefaultConstructible<T>::value;

template <typename T>
struct IsCopyConstructible
    : BoolConstant<std::is_copy_constructible<T>::value> {};
template <typename T>
struct IsTriviallyCopyConstructible
    : BoolConstant<std::is_trivially_copy_constructible<T>::value> {};
template <typename T>
struct IsNothrowCopyConstructible
    : BoolConstant<std::is_nothrow_copy_constructible<T>::value> {};
template <typename T>
inline constexpr auto IsCopyConstructible_V = IsCopyConstructible<T>::value;
template <typename T>
inline constexpr auto IsTriviallyCopyConstructible_V =
    IsTriviallyCopyConstructible<T>::value;
template <typename T>
inline constexpr auto IsNothrowCopyConstructible_V =
    IsNothrowCopyConstructible<T>::value;

template <typename T>
struct IsMoveConstructible
    : BoolConstant<std::is_move_constructible<T>::value> {};
template <typename T>
struct IsTriviallyMoveConstructible
    : BoolConstant<std::is_trivially_move_constructible<T>::value> {};
template <typename T>
struct IsNothrowMoveConstructible
    : BoolConstant<std::is_nothrow_move_constructible<T>::value> {};
template <typename T>
inline constexpr auto IsMoveConstructible_V = IsMoveConstructible<T>::value;
template <typename T>
inline constexpr auto IsTriviallyMoveConstructible_V =
    IsTriviallyMoveConstructible<T>::value;
template <typename T>
inline constexpr auto IsNothrowMoveConstructible_V =
    IsNothrowMoveConstructible<T>::value;

template <typename T, typename U>
struct IsAssignable : BoolConstant<std::is_assignable<T, U>::value> {};
template <typename T, typename U>
struct IsTriviallyAssignable
    : BoolConstant<std::is_trivially_assignable<T, U>::value> {};
template <typename T, typename U>
struct IsNothrowAssignable
    : BoolConstant<std::is_nothrow_assignable<T, U>::value> {};
template <typename T, typename U>
inline constexpr auto IsAssignable_V = IsAssignable<T, U>::value;
template <typename T, typename U>
inline constexpr auto IsTriviallyAssignable_V =
    IsTriviallyAssignable<T, U>::value;
template <typename T, typename U>
inline constexpr auto IsNothrowAssignable_V = IsNothrowAssignable<T, U>::value;

template <typename T>
struct IsCopyAssignable : BoolConstant<std::is_copy_assignable<T>::value> {};
template <typename T>
struct IsTriviallyCopyAssignable
    : BoolConstant<std::is_trivially_copy_assignable<T>::value> {};
template <typename T>
struct IsNothrowCopyAssignable
    : BoolConstant<std::is_nothrow_copy_assignable<T>::value> {};
template <typename T>
inline constexpr auto IsCopyAssignable_V = IsCopyAssignable<T>::value;
template <typename T>
inline constexpr auto IsTriviallyCopyAssignable_V =
    IsTriviallyCopyAssignable<T>::value;
template <typename T>
inline constexpr auto IsNothrowCopyAssignable_V =
    IsNothrowCopyAssignable<T>::value;

template <typename T>
struct IsMoveAssignable : BoolConstant<std::is_move_assignable<T>::value> {};
template <typename T>
struct IsTriviallyMoveAssignable
    : BoolConstant<std::is_trivially_move_assignable<T>::value> {};
template <typename T>
struct IsNothrowMoveAssignable
    : BoolConstant<std::is_nothrow_move_assignable<T>::value> {};
template <typename T>
inline constexpr auto IsMoveAssignable_V = IsMoveAssignable<T>::value;
template <typename T>
inline constexpr auto IsTriviallyMoveAssignable_V =
    IsTriviallyMoveAssignable<T>::value;
template <typename T>
inline constexpr auto IsNothrowMoveAssignable_V =
    IsNothrowMoveAssignable<T>::value;

template <typename T>
struct IsDestructible : BoolConstant<std::is_destructible<T>::value> {};
template <typename T>
struct IsTriviallyDestructible
    : BoolConstant<std::is_trivially_destructible<T>::value> {};
template <typename T>
struct IsNothrowDestructible
    : BoolConstant<std::is_nothrow_destructible<T>::value> {};
template <typename T>
inline constexpr auto IsDestructible_V = IsDestructible<T>::value;
template <typename T>
inline constexpr auto IsTriviallyDestructible_V =
    IsTriviallyDestructible<T>::value;
template <typename T>
inline constexpr auto IsNothrowDestructible_V = IsNothrowDestructible<T>::value;

template <typename T>
struct AlignmentOf : IntegralConstant<SizeT, std::alignment_of<T>::value> {};
template <typename T>
inline constexpr auto AlignmentOf_V = AlignmentOf<T>::value;

template <typename T>
struct Rank : IntegralConstant<SizeT, std::rank<T>::value> {};
template <typename T>
inline constexpr auto Rank_V = Rank<T>::value;

template <typename T, unsigned N = 0>
struct Extent : IntegralConstant<SizeT, std::extent<T, N>::value> {};
template <typename T, unsigned N = 0>
inline constexpr auto Extent_V = Extent<T, N>::value;

template <typename T, typename U>
struct IsSame : BoolConstant<std::is_same<T, U>::value> {};
template <typename T, typename U>
inline constexpr auto IsSame_V = IsSame<T, U>::value;

template <typename Base, typename Derived>
struct IsBaseOf : BoolConstant<std::is_base_of<Base, Derived>::value> {};
template <typename Base, typename Derived>
inline constexpr auto IsBaseOf_V = IsBaseOf<Base, Derived>::value;

template <typename From, typename To>
struct IsConvertible : BoolConstant<std::is_convertible<From, To>::value> {};
template <typename From, typename To>
inline constexpr auto IsConvertible_V = IsConvertible<From, To>::value;

template <typename Fn, typename... Args>
struct IsInvocable : BoolConstant<std::is_invocable<Fn, Args...>::value> {};
template <typename Fn, typename... Args>
inline constexpr auto IsInvocable_V = IsInvocable<Fn, Args...>::value;

template <typename Fn, typename... Args>
struct IsNothrowInvocable
    : BoolConstant<std::is_nothrow_invocable<Fn, Args...>::value> {};
template <typename Fn, typename... Args>
inline constexpr auto IsNothrowInvocable_V =
    IsNothrowInvocable<Fn, Args...>::value;

template <typename R, typename Fn, typename... Args>
struct IsInvocableR : BoolConstant<std::is_invocable_r<R, Fn, Args...>::value> {
};
template <typename Fn, typename... Args>
inline constexpr auto IsInvocableR_V = IsInvocableR<Fn, Args...>::value;

template <typename R, typename Fn, typename... Args>
struct IsNothrowInvocableR
    : BoolConstant<std::is_nothrow_invocable_r<R, Fn, Args...>::value> {};
template <typename Fn, typename... Args>
inline constexpr auto IsNothrowInvocableR_V =
    IsNothrowInvocableR<Fn, Args...>::value;

template <typename T>
struct RemoveConst {
  using type = typename std::remove_const<T>::type;
};
template <typename T>
using RemoveConst_T = typename RemoveConst<T>::type;

template <typename T>
struct RemoveVolatile {
  using type = typename std::remove_volatile<T>::type;
};
template <typename T>
using RemoveVolatile_T = typename RemoveVolatile<T>::type;

template <typename T>
struct RemoveCV {
  using type = typename std::remove_cv<T>::type;
};
template <typename T>
using RemoveCV_T = typename RemoveCV<T>::type;

template <typename T>
struct AddConst {
  using type = typename std::add_const<T>::type;
};
template <typename T>
using AddConst_T = typename AddConst<T>::type;

template <typename T>
struct AddVolatile {
  using type = typename std::add_volatile<T>::type;
};
template <typename T>
using AddVolatile_T = typename AddVolatile<T>::type;

template <typename T>
struct AddCV {
  using type = typename std::add_cv<T>::type;
};
template <typename T>
using AddCV_T = typename AddCV<T>::type;

template <typename T>
struct RemoveReference {
  using type = typename std::remove_reference<T>::type;
};
template <typename T>
using RemoveReference_T = typename RemoveReference<T>::type;

template <typename T>
struct AddLValueReference {
  using type = typename std::add_lvalue_reference<T>::type;
};
template <typename T>
using AddLValueReference_T = typename AddLValueReference<T>::type;

template <typename T>
struct AddRValueReference {
  using type = typename std::add_rvalue_reference<T>::type;
};
template <typename T>
using AddRValueReference_T = typename AddRValueReference<T>::type;

template <typename T>
struct RemovePointer {
  using type = typename std::remove_pointer<T>::type;
};
template <typename T>
using RemovePointer_T = typename RemovePointer<T>::type;

template <typename T>
struct AddPointer {
  using type = typename std::add_pointer<T>::type;
};
template <typename T>
using AddPointer_T = typename AddPointer<T>::type;

template <typename T>
struct MakeSigned {
  using type = typename std::make_signed<T>::type;
};
template <typename T>
using MakeSigned_T = typename MakeSigned<T>::type;

template <typename T>
struct MakeUnsigned {
  using type = typename std::make_unsigned<T>::type;
};
template <typename T>
using MakeUnsigned_T = typename MakeUnsigned<T>::type;

template <typename T>
struct RemoveExtent {
  using type = typename std::remove_extent<T>::type;
};
template <typename T>
using RemoveExtent_T = typename RemoveExtent<T>::type;

template <typename T>
struct RemoveAllExtents {
  using type = typename std::remove_all_extents<T>::type;
};
template <typename T>
using RemoveAllExtents_T = typename RemoveAllExtents<T>::type;

template <typename T>
struct Decay {
  using type = typename std::decay<T>::type;
};
template <typename T>
using Decay_T = typename Decay<T>::type;

template <typename T>
struct RemoveCVRef {
  using type =
      typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};
template <typename T>
using RemoveCVRef_T = typename RemoveCVRef<T>::type;

template <Bool B, typename T = void>
struct EnableIf {};

template <typename T>
struct EnableIf<true, T> {
  using type = T;
};
template <Bool B, typename T = void>
using EnableIf_T = typename EnableIf<B, T>::type;

template <Bool B, typename T, typename F>
struct Conditional {
  using type = typename std::conditional<B, T, F>::type;
};
template <Bool B, typename T, typename F>
using Conditional_T = typename Conditional<B, T, F>::type;

template <typename... Types>
struct CommonType {
  using type = typename std::common_type<Types...>::type;
};
template <typename... Types>
using CommonType_T = typename CommonType<Types...>::type;

template <typename T>
struct UnderlyingType {
  using type = typename std::underlying_type<T>::type;
};
template <typename T>
using UnderlyingType_T = typename UnderlyingType<T>::type;

template <typename Fn, typename... Args>
struct InvokeResult {
  using type = typename std::invoke_result<Fn, Args...>::type;
};
template <typename Fn, typename... Args>
using InvokeResult_T = typename InvokeResult<Fn, Args...>::type;

template <typename...>
using Void_T = void;

template <typename T>
struct TypeIdentity {
  using type = T;
};
template <typename T>
using TypeIdentity_T = typename TypeIdentity<T>::type;

template <typename... Traits>
struct Conjunction : BoolConstant<std::conjunction<Traits...>::value> {};
template <typename... Traits>
inline constexpr auto Conjunction_V = Conjunction<Traits...>::value;

template <typename... Traits>
struct Disjunction : BoolConstant<std::disjunction<Traits...>::value> {};
template <typename... Traits>
inline constexpr auto Disjunction_V = Disjunction<Traits...>::value;

template <typename Trait>
struct Negation : BoolConstant<std::negation<Trait>::value> {};
template <typename... Traits>
inline constexpr auto Negation_V = Negation<Traits...>::value;

template <typename T, typename... Types>
struct IsAnyOf : Disjunction<IsSame<T, Types>...> {};
template <typename T, typename... Types>
inline constexpr auto IsAnyOf_V = IsAnyOf<T, Types...>::value;

template <typename T, typename... Types>
struct IsNoneOf : BoolConstant<!IsAnyOf<T, Types...>::value> {};
template <typename T, typename... Types>
inline constexpr auto IsNoneOf_V = IsNoneOf<T, Types...>::value;

template <typename T, template <typename...> typename Template>
struct IsSpecializationOf : FalseType {};
template <template <typename...> typename Template, typename... Args>
struct IsSpecializationOf<Template<Args...>, Template> : TrueType {};
template <typename T, template <typename...> typename Template>
inline constexpr auto IsSpecializationOf_V =
    IsSpecializationOf<T, Template>::value;

template <typename T, typename = void>
struct IsComplete : FalseType {};
template <typename T>
struct IsComplete<T, Void_T<decltype(sizeof(T))>> : TrueType {};
template <typename T>
inline constexpr auto IsComplete_V = IsComplete<T>::value;

namespace internal {
template <template <typename...> typename Op, typename, typename... Args>
struct IsDetectedImpl : FalseType {};
template <template <typename...> typename Op, typename... Args>
struct IsDetectedImpl<Op, Void_T<Op<Args...>>, Args...> : TrueType {};
}  // namespace internal
template <template <typename...> typename Op, typename... Args>
struct IsDetected : internal::IsDetectedImpl<Op, void, Args...> {};
template <template <typename...> typename Op, typename... Args>
inline constexpr auto IsDetected_V = IsDetected<Op, Args...>::value;

template <typename... Types>
struct TypeList {};

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
template <typename Lhs, typename Rhs = Lhs>
inline constexpr auto IsEqualityComparable_V =
    IsEqualityComparable<Lhs, Rhs>::value;

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
template <typename Lhs, typename Rhs = Lhs>
inline constexpr auto IsLessThanComparable_V =
    IsLessThanComparable<Lhs, Rhs>::value;

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
template <typename Lhs, typename Rhs = Lhs>
inline constexpr auto IsGreaterThanComparable_V =
    IsGreaterThanComparable<Lhs, Rhs>::value;

template <typename T>
struct ShouldUseEBO : BoolConstant<IsEmpty<T>::value && IsClass<T>::value &&
                                   !IsFinal<T>::value> {};
template <typename T>
inline constexpr auto ShouldUseEBO_V = ShouldUseEBO<T>::value;

template <typename T>
struct UnwrapReferenceWrapper {
  using type = T;
};

template <typename T>
struct UnwrapReferenceWrapper<std::reference_wrapper<T>> {
  using type = T&;
};
template <typename T>
using UnwrapReferenceWrapper_T = typename UnwrapReferenceWrapper<T>::type;
}  // namespace axio

#endif