import matplotlib.pyplot as plt
import scienceplots
import seaborn as sns
import numpy as np
import os


def main():

    i_arr = np.arange(1.0,56.0,1.0)

    # get maximum recoil energy to make the mesh grid
    max_data = np.loadtxt('diff_xs_nu_energy_55.0.out')
    energy = max_data[:,0] 
    step_size = energy[1] - energy[0]


    # Want a transfer matrix with neutrino energy on the x-axis 
    # and on the y-axis the recoil energy

    # The max recoil energy is energy[-1] and the min is energy[0]
    # The max neutrino energy is 55.0 and the min is 1.0

    Z = np.zeros((len(energy),len(i_arr)))

    for i in range(len(i_arr)):
        data = np.loadtxt('diff_xs_nu_energy_{}.out'.format(i_arr[i]))
        
        recoil_energy = data[:,0]
        diff_xs = data[:,1]

        for j in range(len(recoil_energy)):
            Z[-j,i] = diff_xs[j]


    # Plot the matrix
    plt.figure(figsize=(14,10))

    offset = step_size

    ax = sns.heatmap(Z[::-1], cmap='viridis', 
                        cbar_kws={'label': 'Differential cross section [cm^2/MeV/atom]'})
    
    ax.invert_yaxis()  

    ax.set_xticks(np.arange(0, len(i_arr), 10))
    ax.set_xticklabels(i_arr[::10])

    ax.set_yticks(np.arange(0, len(energy), 500))
    ax.set_yticklabels(1000*energy[::500])

    ax.set_xlabel('Neutrino energy [MeV]')
    ax.set_ylabel('Recoil energy [keV]')
    ax.set_title('Flux/Recoil Transfer Matrix')
    plt.savefig('transfer.png', dpi=300)

    
   




if __name__ == "__main__":
    main()