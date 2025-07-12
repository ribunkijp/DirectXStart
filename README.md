### 

```c++
// |  ^   & ~
// | and
WS_THICKFRAME | WS_MINIMIZE
// ^
0 0 -> 0
1 1 -> 0
0 1 -> 1
1 0 -> 1
WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX) WS_OVERLAPPEDWINDOWの中に(WS_THICKFRAME | WS_MAXIMIZEBOX)があったら削除、なかったら追加
//　& ~　削除
WS_OVERLAPPEDWINDOW & ~ (WS_THICKFRAME | WS_MINIMIZE and)
    
    
//ポインター　タイプ
RECT*  rect;  // Pointer to a RECT structure.
LPRECT rect;  // The same
PRECT  rect;  // Also the same.

//BOOLは整数,c++のboolと違う

//ウィンドウ　ハンドル
//ウィンドウで操作を実行するには、通常、 HWND 値をパラメーターとして受け取る関数を呼び出します
HWND hWnd 




```



### to .dds

```c++
texconv -ft DDS -f BC3_UNORM texture.png
texconv -f BC3_UNORM -o . 1.png
```



