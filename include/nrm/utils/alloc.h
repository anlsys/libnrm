/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the libnrm project.
 * For more info, see https://github.com/anlsys/libnrm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/

#ifndef NRM_ALLOCS_H
#define NRM_ALLOCS_H 1

// Stringify macro
#define STRINGIFY(a) STRINGIFY_(a)
#define STRINGIFY_(a) #a

// Concatenate two arguments into a macro name
#define CONCATENATE(arg1, arg2) CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2) CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2) arg1##arg2

// Expand to number of variadic arguments for up to 36 args.
// The last argument `_` is here to avoid having empty __VA_ARGS__
// in PP_ARG_N().
#define VA_NARG(...)                                                           \
	PP_ARG_N(__VA_ARGS__, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25,  \
	         24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,   \
	         9, 8, 7, 6, 5, 4, 3, 2, 1, _)
#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14,  \
                 _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,   \
                 _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, N, ...)     \
	N

// Arithmetic
#define PLUS_1_0 1
#define PLUS_1_1 2
#define PLUS_1_2 3
#define PLUS_1_3 4
#define PLUS_1_4 5
#define PLUS_1_5 6
#define PLUS_1_6 7
#define PLUS_1_7 8
#define PLUS_1_8 9
#define PLUS_1_9 10
#define PLUS_1_10 11
#define PLUS_1_11 12
#define PLUS_1_12 13
#define PLUS_1_13 14
#define PLUS_1_14 15
#define PLUS_1_15 16
#define PLUS_1_16 17
#define PLUS_1_17 18
#define PLUS_1_18 19
#define PLUS_1_19 20
#define PLUS_1_20 21
#define PLUS_1_21 22
#define PLUS_1_22 23
#define PLUS_1_23 24
#define PLUS_1_24 25
#define PLUS_1_25 26
#define PLUS_1_26 27
#define PLUS_1_27 28
#define PLUS_1_28 29
#define PLUS_1_29 30
#define PLUS_1_30 31
#define PLUS_1_31 32
#define PLUS_1_32 33
#define PLUS_1_33 34
#define PLUS_1_34 35
#define PLUS_1_35 36
#define PLUS_1_36 37
#define PLUS_1(N) CONCATENATE(PLUS_1_, N)


// Field name in struct: __f1 for N = 1
#define NRM_FIELD(N) CONCATENATE(__f, N)
#define NRM_FIELD_DECL(type, N) type NRM_FIELD(N);

/* struct fields declaration.
 * one field: f1 __f1;
 * two fields: f2 __f1; f1 __f2;
 * three fields: f3 __f1; f2 __f2; f1 __f3;
 * We want fx fields to appear in the order of types provided by users.
 * We want __fx names to appear in the reverse order, such that if the user
 * wants the second field it can name it with __f2.
 */
#define NRM_DECL_1(N, t, ...) NRM_FIELD_DECL(t, N)
#define NRM_DECL_2(N, t, ...)                                                  \
	NRM_FIELD_DECL(t, N) NRM_DECL_1(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_3(N, t, ...)                                                  \
	NRM_FIELD_DECL(t, N) NRM_DECL_2(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_4(N, t, ...)                                                  \
	NRM_FIELD_DECL(t, N) NRM_DECL_3(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_5(N, t, ...)                                                  \
	NRM_FIELD_DECL(t, N) NRM_DECL_4(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_6(N, t, ...)                                                  \
	NRM_FIELD_DECL(t, N) NRM_DECL_5(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_7(N, t, ...)                                                  \
	NRM_FIELD_DECL(t, N) NRM_DECL_6(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_8(N, t, ...)                                                  \
	NRM_FIELD_DECL(t, N) NRM_DECL_7(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_9(N, t, ...)                                                  \
	NRM_FIELD_DECL(t, N) NRM_DECL_8(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_10(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_9(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_11(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_10(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_12(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_11(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_13(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_12(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_14(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_13(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_15(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_14(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_16(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_15(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_17(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_16(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_18(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_17(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_19(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_18(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_20(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_19(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_21(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_20(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_22(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_21(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_23(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_22(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_24(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_23(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_25(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_24(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_26(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_25(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_27(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_26(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_28(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_27(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_29(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_28(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_30(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_29(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_31(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_30(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_32(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_31(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_33(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_32(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_34(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_33(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_35(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_34(PLUS_1(N), __VA_ARGS__)
#define NRM_DECL_36(N, t, ...)                                                 \
	NRM_FIELD_DECL(t, N) NRM_DECL_35(PLUS_1(N), __VA_ARGS__)

/* Declare a structure with up to 36 fields.
 * (Pick the adequate NRM_DECL_ macro and call it.)
 */
#define NRM_STRUCT_DECL(...)                                                   \
	struct {                                                               \
		CONCATENATE(NRM_DECL_, VA_NARG(__VA_ARGS__))                   \
		(1, __VA_ARGS__, 0)                                            \
	}

/** Returns the size required for allocation **/
#define NRM_SIZEOF_ALIGNED(...) sizeof(NRM_STRUCT_DECL(__VA_ARGS__))

/** Returns the offset of the nth type of a list **/
#define NRM_OFFSETOF_ALIGNED(N, ...)                                           \
	offsetof(NRM_STRUCT_DECL(__VA_ARGS__), NRM_FIELD(N))

/* Allocation ifself */
#define NRM_INNER_MALLOC(...) calloc(1, NRM_SIZEOF_ALIGNED(__VA_ARGS__))

/**
 * Returns the nth __VA__ARGS__ field pointer from NRM_INNER_MALLOC*()
 * allocation.
 * @param ptr: A pointer obtained from NRM_INNER_MALLOC*()
 * @param N: The field number. N must be a number (1, 2, 3, 4, 5, 6, 7, 8)
 * and not a variable.
 * @param ...: types contained in allocation. (Up to 8)
 * @return A pointer to Nth field after ptr.
 **/
#define NRM_INNER_MALLOC_GET_FIELD(ptr, N, ...) \
	(void *)(((intptr_t) ptr) + NRM_OFFSETOF_ALIGNED(N, __VA_ARGS__))


#endif
