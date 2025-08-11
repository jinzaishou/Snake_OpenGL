# 阶段1
	环境配置，使用glfw和glad在显示出窗口
# 阶段2
	可视化蛇并且实现蛇的移动
# 阶段3
	添加蛇吃食物变长的逻辑，绘制了蛇头，蛇身和食物的纹理。增加了蛇头旋转的逻辑。
# 注意事项
	glad(graphics libiary Loader and dependcy manager)、glfw(graphics libiary framwork for windows)
	主函数中导入库需要先导入glad.h，再导入glfw3.h
	绘制像素纹理的网址：https://www.piskelapp.com/
	
# 常用的函数说明：



# 学习笔记：
	opengl是一个状态机
	使用双缓冲区机制来进行绘图，具体代码：glfwSwapBuffers(window)//用于交换前后缓冲区。
	使用 location = 1、2、3 是在顶点有多个属性（位置、颜色、纹理坐标等）时，为了明确告诉 OpenGL：“每个输入变量对应哪个通道”。使用glVertexAttribPointer()绑定通道，使用glEnableVertexAttribArray()启用顶点属性。


# opengl渲染的流程
	       [程序初始化阶段]
┌──────────────────────────┐
│ glGenVertexArrays(...)   │ ◀───── 创建 VAO
│ glGenBuffers(...)        │ ◀───── 创建 VBO
│ glCreateShader(...)      │ ◀───── 创建顶点 & 片元着色器
└──────────────────────────┘
              │
              ▼
       [着色器阶段]
┌──────────────────────────┐
│ glShaderSource(...)       │ ◀──── 加载源码
│ glCompileShader(...)      │ ◀──── 编译
│ glAttachShader(...)       │
│ glLinkProgram(...)        │ ◀──── 链接成 program
└──────────────────────────┘
              │
              ▼
       [VAO/VBO配置阶段]
┌───────────────────────────────────────────────┐
│ glBindVertexArray(VAO)                        │ ◀──── 绑定 VAO（记录状态）★
│   glBindBuffer(GL_ARRAY_BUFFER, VBO)          │ ◀──── 绑定 VBO（传数据）★
│   glBufferData(...)                           │ ◀──── 把顶点数据送进 VBO
│   glVertexAttribPointer(location, ...)        │ ◀──── 告诉 VAO 数据怎么解读
│   glEnableVertexAttribArray(location)         │ ◀──── 启用通道
└───────────────────────────────────────────────┘
              │
              ▼
       [每帧渲染阶段]
┌───────────────────────────────────────────────┐
│ glUseProgram(shaderProgram)                   │ ◀──── 使用着色器 ★
│ glBindVertexArray(VAO)                        │ ◀──── 绑定 VAO ★
│ glUniformXXX(...)                             │ ◀──── 设置 uniform（如位置、颜色）★
│ glDrawArrays(...) or glDrawElements(...)      │ ◀──── 绘制
└───────────────────────────────────────────────┘


	