**ShoutVST** is a VST that enables streaming sound into Icecast/ShoutCast directly from VST hosts, ideal for streaming live performances directly from applications like **Traktor** or **Ableton** without the use of loopback methods. Only Windows is supported at the moment. 
You can try that precompiled binary (Just copy it into your VST plugins folder):

<a href="https://www.kvraudio.com/product/shoutvst-by-r-tur">
<img src="http://jsound.org/img/download.png?1" alt="ShoutVST Download" height="175">
</a>

![ShoutVST](https://static.kvraudio.com/i/b/shoutvst.jpg "ShoutVST")

#Prerequisites#

[Visual Studio 2013+](https://www.visualstudio.com/downloads/download-visual-studio-vs)

[CMake >=3.0](https://cmake.org/download/)

[GIT](https://git-scm.com/download/win)

#Build#

```
git clone https://github.com/Iunusov/ShoutVST.git
```

```
cd ShoutVST
```

```
!!make_project!!.bat
```

#Build (x64)#

```
mkdir build64
```

```
cd build64
```

```
cmake -G "Visual Studio 14 2015 Win64" ..
```

#Linux Build (e.g. UBUNTU)#

```
sudo apt-get install build-essential
sudo apt-get install libx11-dev
```

```
cd deps
./sync.sh
cd ..
```

```
mkdir build
cd build
cmake ..
make
```

#The Result#

After successfull build the resulting DLL file will be available there (you can copy it into your VST folder):
```
ShoutVST\ShoutVST_DLL
```

#Tested Hosts#

Cubase (5.1.1), Reaper (5.22), FL Studio (11.1.0), LMMS (1.1.3)
