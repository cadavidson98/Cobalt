import argparse
import colour
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import numpy as np

def plotCIE(filename, spectra):
    with open(filename) as ciefile:
        l, x, y, z = np.loadtxt(ciefile, delimiter=',', unpack=True)

    fig, ax = plt.subplots(layout='constrained')
    # ax.plot(l, x, 'r', l, y, 'g', l, z, 'b')

    # ax.set(xlabel='wavelength')

    if spectra is not None:
        with open(spectra) as csvfile:
            l_s, x_s = np.loadtxt(spectra, delimiter=',', unpack=True)
        x_s = np.interp(l, l_s, x_s)

        y_integral = 106.856895

        cie_x = np.dot(x, x_s) / y_integral
        cie_y = np.dot(y, x_s) / y_integral
        cie_z = np.dot(z, x_s) / y_integral

        rgb = colour.XYZ_to_RGB(np.array([cie_x, cie_y, cie_z]), colour.models.RGB_COLOURSPACE_sRGB)

        ax.add_patch(Rectangle((1, 1), 1, 1, color=rgb))
        ax.add_patch(Rectangle((2.5, 1), 1, 1, color=[.0858, .3465, .7368]))

        ax.set_xlim(0, 5)
        ax.set_ylim(0, 2)

        print(f"CIE color is {rgb}")

        # ax.plot(l, x_s, 'k')

    # ax.axis('off')

    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
                    prog='CSVPlotter',
                    description='Plots CIE CSV files for debug vis')
    parser.add_argument('filename', help='CSV file to read')
    parser.add_argument('--spectrum', help='CSV file of color spectra data')
    args = parser.parse_args()
    plotCIE(args.filename, args.spectrum)