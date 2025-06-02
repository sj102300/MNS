# 📦 ScenarioManager 정적 라이브러리 사용 가이드

이 문서는 `ScenarioManager` 모듈을 **정적 라이브러리 (`.lib`)로 빌드**하고, 이를 **외부 프로젝트에서 사용하는 방법**을 설명합니다.

---

## 🧱 1. 정적 라이브러리 빌드 방법

### 📁 프로젝트 구조 예시

```
/ScenarioManager/
├── include/
│   ├── ScenarioManager.h
│   ├── HttpServer.h
│   └── ...
├── src/
│   ├── ScenarioManager.cpp
│   ├── HttpServer.cpp
│   └── ...
├── ScenarioManager.vcxproj  # 정적 라이브러리 프로젝트
```

### ⚙️ Visual Studio에서 빌드하기

1. `ScenarioManager` 프로젝트를 **정적 라이브러리 (/MT 또는 /MD)**로 설정합니다.  
   - **속성 > 일반 > 구성 형식**: `정적 라이브러리 (.lib)`
2. `Debug` 또는 `Release` 구성으로 빌드합니다. (`Ctrl + Shift + B`)
3. 빌드 후 `ScenarioManager.lib` 파일이 `Debug/` 또는 `Release/` 디렉토리에 생성됩니다.

---

## 🧩 2. 외부 프로젝트에서 사용하는 방법

### 📁 외부 프로젝트 구조 예시

```
/MyApp/
├── main.cpp
├── MyApp.vcxproj
└── ...
```

### ✅ 설정 방법 (Visual Studio 기준)

#### 📌 C/C++ > 일반
- **추가 포함 디렉터리**  
  ```
  ../ScenarioManager/include
  ```

#### 📌 링커 > 일반
- **추가 라이브러리 디렉터리**  
  ```
  ../ScenarioManager/Debug    # 또는 Release
  ```

#### 📌 링커 > 입력
- **추가 종속성**  
  ```
  ScenarioManager.lib
  ```

#### 📌 C/C++ > 전처리기
- 다음 플래그가 정의되어 있어야 합니다 (cpprestsdk 호환 목적):  
  ```
  UNICODE
  _UNICODE
  ```

---

## 🧪 3. 간단한 사용 예시

```cpp
#include "ScenarioManager.h"

int main() {
    sm::ScenarioManager manager("http://localhost:8080", "http://localhost:9000", "MFR");
    manager.run();  // 블로킹 호출
    return 0;
}
```

---

## ⚠️ 4. 유의사항

- 이 라이브러리는 **cpprestsdk (Casablanca)** 를 기반으로 하므로 해당 라이브러리가 설치되어 있어야 합니다.
- 콘솔 출력을 UTF-8로 설정하려면 다음 코드를 호출하세요:
  ```cpp
  SetConsoleOutputCP(CP_UTF8);
  ```
- `ScenarioManager::run()`은 **블로킹 함수**입니다. 별도의 스레드에서 실행하거나 메인 루프와 병렬로 동작시키는 것이 일반적입니다.
- 콜백 함수(`setOnReadyCallback`, `setOnQuitCallback`)는 멀티스레드 환경에서 안전하게 작성해야 합니다.

---
