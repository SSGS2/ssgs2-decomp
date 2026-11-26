/* Mark the original PowerPC single-precision fused operations (a*c +/- b): fmadds, fmsubs, fnmadds, fnmsubs.
 * The expressions keep CodeWarrior's contraction (-fp_contract on) and so the original instruction stream.
 * ADD_FIRST/SUB_FIRST keep the source operand order; ACCUM updates the accumulator in place. */
#ifndef FP_FUSED_H
#define FP_FUSED_H

#define FP_FMADDS(a, c, b) ((a) * (c) + (b))
#define FP_FMSUBS(a, c, b) ((a) * (c) - (b))
#define FP_FNMADDS(a, c, b) (-((a) * (c) + (b)))
#define FP_FNMSUBS(a, c, b) (-((a) * (c) - (b)))
#define FP_FNMSUBS_SUB_FIRST(b, a, c) ((b) - (a) * (c))
#define FP_FMADDS_ADD_FIRST(b, a, c) ((b) + (a) * (c))
#define FP_FMADDS_ACCUM(b, a, c) ((b) += (a) * (c))
#define FP_FNMSUBS_ACCUM(b, a, c) ((b) -= (a) * (c))

#endif /* FP_FUSED_H */
