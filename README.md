# Wavelet-transform
Wavelet 변환을 구현하고 이를 활용하여 이미지 압축과 복원을 해보며 결과를 비교/분석한다.

## Project 개요

* 푸리에 변환을 사용하기 위해서는 시간(공간)영역의 무한한 구간의 정보에 대해 알고 있어야 한다.

* 또한 푸리에 변환시, 특정 부분에서 어떤 주파수가 나오는지에 대한 정보를 알 수 있는 길이 없다. 그저 어느 주파수들이 신호에서 어느정도 존재하는지만 알 수 있을 뿐이다.

* 신호의 구간별로 주파수 영역에 대한 정보를 얻어올 수 있는 방법에 대해 알아보고, 직접 구현해보자.


## 이론 탐구

  ### Short Time Fourier Transform
  
  ![image](https://user-images.githubusercontent.com/67624104/118353780-cfc42600-b5a2-11eb-8e2b-c708f31e5d1d.png)

  * 신호의 구간별로 주파수를 얻어올 수 있는 방법의 첫번째로 short time fourier transform이 있다.

  * Window function을 정보를 추출하기를 원하는 구간으로 평행이동 하여 푸리에 변환을 하는 방식

  * 특정 window 내의 주파수 정보를 얻어올 수 있긴 하지만, window size가 크면 시간의 경계가 애매해지고 size가 작으면 주파수의 정확도가 낮아진다.

  * 이러한 한계점을 극복하기 위해 wavelet transform을 사용해볼 수 있다.



  ### Wavelet transform

  ![image](https://user-images.githubusercontent.com/67624104/118353849-3e08e880-b5a3-11eb-8f8a-457adcac18f9.png)

  * 다양한 scale의 wavelet function을 사용하여 구간이 짧든 길든 정확한 주파수 정보를 가져올 수 있다.

  * 주의할 점은 wavelet transform이 다루는 주파수는 strict한 주파수는 아니다. 정해준 스케일로 wavelet function을 scaling한 후 이 function과 구하려는 구간의 신호가 얼마나 닮아있는지를 나타낼 뿐이다.

  * 앞으로 Haar wavlelet function을 사용해서 프로젝트를 진행해 볼 예정이다.



---


## 프로젝트 내용

* 2-tap 1-d filtering 수행

![image](https://user-images.githubusercontent.com/67624104/118354039-17977d00-b5a4-11eb-9867-a1c45bb0c876.png)

* Scaling function이 low pass filter의 역할을 하는 이유 : 주기가 무한인 비주기 함수이므로 주파수가 0 인 DC로 생각할 수 있기 때문이다.

* Wavelet function이 high pass filter의 역할을 하는 이유 : Haar wavelet function의 경우 두 픽셀의 크기의 차이가 클수록 값이 크고 작을수록 작게 되도록 설계되어 있다. 이는 곧 high pass filter임을 의미한다.

* Coefficient 값이 1/2, -1/2인 이유: 이 부분을 도출하기 위해서는 inverse transform도 분명히 고려되어야 한다. 일단 저렇게 설정하면 inverse transform시 dc에서 평균을 더해주고 빼줌으로써 한 번에 두개의 픽셀을 복원할 수 있다. 그러나 tap 수가 많아지거나 wavelet function이 달라질수록 계수들과 scaling function이 복잡해질 수 있는 가능성은 항상 열려있다. 당장 tap을 4로 늘려봐도 현재 방법으로는 inverse transform이 불가능한 것을 알 수 있다.

* Quantization 방식은 uniquantization을 사용하겠다. 그러나 고려해야 할 점은 이제 소수점 단위 픽셀값까지 다루어야 한다는 것이다. 따라서 encoding시 일일이 소수점 값들을 계산해가며 양자화 하지 않고 축적해놨다가 decoding할 때 한번에 나눗셈 계산을 하는 방향으로 진행하였다.


---

## Result images(Original - Uniquantization - 7,7,6,5 wavelet - 7,7,5,4 wavelet)

* PSNR

![image](https://user-images.githubusercontent.com/67624104/118354431-fcc60800-b5a5-11eb-87fe-6e4ffbe2155f.png)


* 7-7-6-5 transformed image

![image](https://user-images.githubusercontent.com/67624104/118354488-44e52a80-b5a6-11eb-9d98-5b6dadcb11b7.png)


* 7-7-6-5 reconstructed image

![image](https://user-images.githubusercontent.com/67624104/118354549-8675d580-b5a6-11eb-8c1d-9102c4f34b8d.png)


