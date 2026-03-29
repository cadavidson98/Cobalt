import argparse
import colour
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import numpy as np
import math

def blackbodySpectra(temp, l):
    c = 299792458.0
    h = 6.62606957e-34
    k_b = 1.3806488e-23
    numerator = 2.0 * h * c * c
    denominator = np.pow(l, 5) * (np.exp((h * c) / (k_b * l * temp)) - 1.0) 
    return numerator / denominator

def spectraToRGB(x, y, z, s):
    y_integral = 106.856895

    cie_x = np.dot(x, s) / y_integral
    cie_y = np.dot(y, s) / y_integral
    cie_z = np.dot(z, s) / y_integral

    rgb = colour.XYZ_to_RGB(np.array([cie_x, cie_y, cie_z]), colour.models.RGB_COLOURSPACE_sRGB)
    return np.clip(rgb, 0, 1)

def plotCIE(filename, spectra):
    with open(filename) as ciefile:
        l, x, y, z = np.loadtxt(ciefile, delimiter=',', unpack=True)
        t = np.row_stack((l, x, y, z))
        np.savetxt("/home/cole/Downloads/cie.csv", t, fmt='%6f', delimiter=',')

    fig, ax = plt.subplots(layout='constrained')
    # ax.plot(l, x, 'r', l, y, 'g', l, z, 'b')

    # ax.set(xlabel='wavelength')

    cmfs = (colour.MSDS_CMFS["CIE 1931 2 Degree Standard Observer"].copy().align(colour.SpectralShape(360, 780, 10)))

    illuminant = colour.SDS_ILLUMINANTS["D65"].copy().align(cmfs.shape)

    blueXYZ = colour.RGB_to_XYZ([0.0, 0.0, 1.0], colour.models.RGB_COLOURSPACE_sRGB)
    spectra = colour.XYZ_to_sd(blueXYZ, 'Jakob 2019', cmfs=cmfs, illuminant=illuminant)

    print(spectra.wavelengths)

    ax.plot(spectra.wavelengths, spectra.values)

    show_blackbody = False
    if spectra is not None:
        s = np.interp(l, spectra.wavelengths, spectra.values, left=0.0, right=0.0)

        rgb = spectraToRGB(x, y, z, s)
        print(f"approximated rgb: {rgb}")

        ax.add_patch(Rectangle((1, 1), 1, 1, color=rgb))
        ax.add_patch(Rectangle((2.5, 1), 1, 1, color=[.0858, .3465, .7368]))

        ax.set_xlim(0, 5)
        ax.set_ylim(0, 2)

        print(f"CIE color is {rgb}")
    elif show_blackbody:
        temps = np.linspace(1000, 8000, 500)

        for idx in range(temps.size):
            t = temps[idx]
            l_max = 2.8977721e-3 / t
            radiation = blackbodySpectra(t, l * 1e-9) / blackbodySpectra(t, l_max)
            rgb = spectraToRGB(x, y, z, radiation)
            # print(f"{t} -> {rgb}")
            ax.add_patch(Rectangle((idx, 0), 1, 1, color=rgb))
            # ax.plot(l, radiation)

        ax.set_xlim(0, temps.size + 1)

    plt.show()

def xyYToXYZ(chromaticity):
    return np.array([chromaticity[0] / chromaticity[1], 1, (1 - chromaticity[0] - chromaticity[1]) / chromaticity[1]])

def xyzToRGB():
    primaries = np.array([[0.64, 0.30, 0.15], [0.33, 0.60, 0.06], [0.03, 0.10, 0.79]])
    whitepoint = xyYToXYZ(np.array([0.3127, 0.3290]))

    f = np.linalg.inv(primaries) @ whitepoint
    rgb_to_xyz = primaries @ np.diag(f)
    xyz_to_rgb = np.linalg.inv(rgb_to_xyz)
    print("===== Derived =====")
    print(f"foo: {f}")
    print(f"primaries: {primaries}")
    print(f"whitepoint: {whitepoint}")
    print(f"rgb_to_xyz: {rgb_to_xyz}")
    print(xyz_to_rgb)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
                    prog='CSVPlotter',
                    description='Plots CIE CSV files for debug vis')
    #parser.add_argument('filename', help='CSV file to read')
    #parser.add_argument('--spectrum', help='CSV file of color spectra data')
    #args = parser.parse_args()
    #plotCIE(args.filename, args.spectrum)

    print("===== Reference =====")
    illuminant = np.array([0.3127, 0.3290])
    sRGB = colour.models.RGB_COLOURSPACE_DCI_P3
    print(f"primaries: {sRGB.primaries}")
    print(f"whitepoint: {sRGB.whitepoint}")
    print(f"rgb_to_xyz: {sRGB.matrix_RGB_to_XYZ}")
    print(f"rgb_to_xyz: {sRGB.matrix_XYZ_to_RGB}")
    xyzToRGB()