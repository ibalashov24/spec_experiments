import numpy as np
import matplotlib.pyplot as plt

spec_all = [8025]
spec_length = [8137]
spec_matrix = [9126]
no_spec = [7526]

ind = np.arange(1)

width = 0.3 
plt.bar(ind, no_spec, width, label="No spec")
plt.bar(ind + width, spec_length, width, label="Spec length")
plt.bar(ind + 2 *  width, spec_matrix, width, label="Spec matrix")
plt.bar(ind + 3 * width, spec_all, width, label="Spec all")

plt.ylabel("CPU time, ns")
plt.title("Sparse matrix product (COO, CPU, JIT-Impala)")

plt.xticks(ind + 1.5 * width, ["fs1831 x eye3"])
plt.legend(loc="best")
plt.show()
