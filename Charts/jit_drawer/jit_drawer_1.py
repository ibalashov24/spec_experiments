import numpy as np
import matplotlib.pyplot as plt

spec_all = [172553]
spec_length = [172993]
spec_matrix = [234732]
no_spec = [3569461]

ind = np.arange(1)

width = 0.3 
plt.bar(ind, no_spec, width, label="No spec")
plt.bar(ind + width, spec_length, width, label="Spec length")
plt.bar(ind + 2 *  width, spec_matrix, width, label="Spec matrix")
plt.bar(ind + 3 * width, spec_all, width, label="Spec all")

plt.ylabel("CPU time, ns")
plt.title("Sparse matrix product (COO, CPU, JIT-Impala)")

plt.xticks(ind + 1.5 * width, ["bcsstk16 x 2blocks"])
plt.legend(loc="best")
plt.show()
