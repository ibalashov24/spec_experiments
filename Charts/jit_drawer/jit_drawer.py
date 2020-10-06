import numpy as np
import matplotlib.pyplot as plt

spec_all = [172553, 20487, 94126, 8025]
spec_length = [172993, 20594, 94993, 8137]
spec_matrix = [234732, 23063, 142712, 9126]
no_spec = [3756835, 48082, 152191, 7526]
 
ind = np.arange(4)

width = 0.15
plt.bar(ind, no_spec, width, label="No spec")
plt.bar(ind + width, spec_length, width, label="Spec length")
plt.bar(ind + 2 *  width, spec_matrix, width, label="Spec matrix")
plt.bar(ind + 3 * width, spec_all, width, label="Spec all")

plt.ylabel("CPU time, ns")
plt.title("Sparse matrix product (COO, CPU, JIT-Impala)")

plt.xticks(ind + width / 2, ("bcsstk16 x 2blocks", "fs1831 x 2blocks", "bcsstk16 x eye3", "fs1831 x eye3"))
plt.legend(loc="best")
plt.show()
