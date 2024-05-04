import matplotlib.pyplot as plt
import scienceplots
import seaborn as sns
import numpy as np
import os


def main():

    klein = np.loadtxt('../out/diff_rates_alliso-mono-Ge-klein.out')
    unity = np.loadtxt('../out/diff_rates_alliso-mono-Ge-unity.out')

    e = klein[:,0] * 1e3
    klein_diff_xs = klein[:,1]
    unity_diff_xs = unity[:,1]

    ratio = klein_diff_xs/unity_diff_xs

    Z = np.zeros((len(e),len(e)))

    for i in range(len(e)):
        index = (np.abs(e - e[i]*ratio[i])).argmin()

        Z[-index,i] = 1

    plt.figure(figsize=(14,10))
    plt.imshow(Z, cmap='viridis', aspect='auto', extent=[1.0,55.0,1.0,55.0])
    plt.colorbar(label='Ratio of differential cross sections')
    plt.xlabel('True Recoil Energy [keV]')
    plt.ylabel('FF Recoil Energy [keV]')
    plt.title('Klein/Unity Transfer Matrix')
    plt.savefig('transfer.png', dpi=300)



    # # make a matrix
    # Z = np.zeros((len(e),len(e)))
    # for i in range(len(e)):
    #     for j in range(len(e)):
    #         Z[i,j] = ratio[j]

    # plt.figure(figsize=(14,10))
    # plt.imshow(Z, cmap='viridis', aspect='auto', extent=[1.0,55.0,1.0,55.0])
    # plt.colorbar(label='Ratio of differential cross sections')
    # plt.xlabel('Recoil Energy [keV]')
    # plt.ylabel('Recoil Energy [keV]')
    # plt.title('Klein-Nishina/Unity Transfer Matrix')
    # plt.savefig('transfer.png', dpi=300)

    
   




if __name__ == "__main__":
    main()