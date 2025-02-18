## Note
The detection method is currently being transitioned from a predominantly edge- and contour-based approach to a cluster-based approach. As a result, the presentation and statistics found in this folder are no longer fully up-to-date, although some core concepts remain relevant.

# Darts Detection - Theoretical Foundations

## Current Detection Method
In the current detection approach, the dart is not considered as an object with a spatial position but rather as a symmetrical cluster with an orientation in a two-dimensional space. By applying Principal Component Analysis (PCA), a centroid of the dart cluster and a principal axis can be determined. The goal of this method is to achieve significantly higher robustness compared to previous detection techniques. Previously, minor object variations such as strong patterns or small contours could lead to weak detection.

To improve symmetry and enhance detection, the image is processed using a large Gaussian filter and morphological operations. These steps help to remove noise pixels and close dart contours, increasing the symmetry of the detected cluster.

The principal axis is determined in a two-step process:
1. First, a Region of Interest (ROI) is defined where the dart is located to minimize interference from noise pixels. Within this ROI, a centroid and an initial principal axis are computed.
2. Second, a narrower ROI is placed around the detected principal axis and centroid. A new PCA is then performed within this refined region, further reducing the influence of noise pixels outside the dart.

The idea of using the principal axis works particularly well when the dart appears highly symmetrical, ensuring that the axis runs centrally through the dart. However, this new method has not yet been fully tested.

The old edge- and contour-based detection method is still present in the source code but commented out.

## Additional Improvements: Dartboard Calibration
Another aspect currently being tested involves the calibration of dartboards, specifically transforming a side-perspective image into an ideal round shape. The goal is to use a "perfect" reference image that can help adjust slight variations in the dartboard's position by automatically determining and modifying the source points for warping.

## Future Plans
A more comprehensive documentation of the entire system is planned for the future, but further evaluations and analyses are required first. Until then, the partially outdated presentation will remain available as a reference.
