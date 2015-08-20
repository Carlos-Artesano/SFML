
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>

#define SHADER_SOURCE(...) #__VA_ARGS__

#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1
#define TEX_SIZE		128

GLuint CompileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLog.size(), NULL, &infoLog[0]);

        std::cerr << "shader compilation failed: " << &infoLog[0];

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource)
{
    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == 0)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, infoLog.size(), NULL, &infoLog[0]);

        std::cerr << "program link failed: " << &infoLog[0];

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // Request a 24-bits depth buffer when creating the window
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 24;
    contextSettings.majorVersion = 2;
    contextSettings.minorVersion = 0;
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML graphics with OpenGL", sf::Style::Default, contextSettings);
    window.setVerticalSyncEnabled(true);

    // Create a sprite for the background
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("resources/background.jpg"))
        return EXIT_FAILURE;
    sf::Sprite background(backgroundTexture);

    // Create some text to draw on top of our OpenGL object
    sf::Font font;
    if (!font.loadFromFile("resources/sansation.ttf"))
        return EXIT_FAILURE;
    sf::Text text("SFML / OpenGL demo", font);
    text.setColor(sf::Color(255, 255, 255, 170));
    text.setPosition(250.f, 450.f);

    // Make the window the active target for OpenGL calls
    // Note: If using sf::Texture or sf::Shader with OpenGL,
    // be sure to call sf::Texture::getMaximumSize() and/or
    // sf::Shader::isAvailable() at least once before calling
    // setActive(), as those functions will cause a context switch
    window.setActive();

    const GLubyte* version = glGetString(GL_VERSION);
    GLuint mProgram;
    GLuint mProgramCube;
        std::string vs = SHADER_SOURCE
        (
            attribute vec4 vPosition;
            void main()
            {
                gl_Position = vPosition;
            }
        );

        std::string fs = SHADER_SOURCE
        (
            precision mediump float;
            void main()
            {
                gl_FragColor = vec4(0.5, 0.0, 0.5, 1.0);
            }
        );

        mProgram = CompileProgram(vs, fs);
        
	// Fragment and vertex shaders code
        fs = SHADER_SOURCE 
        (
		    uniform sampler2D sampler2d;
		    varying mediump vec2	myTexCoord;
		    void main (void)
		    {
		        gl_FragColor = texture2D(sampler2d,myTexCoord);
		    }
        );
        vs = SHADER_SOURCE
        (
		    attribute highp vec4	myVertex;
		    attribute mediump vec4	myUV;
		    uniform mediump mat4	myPMVMatrix;
		    varying mediump vec2	myTexCoord;
		    void main(void)
		    {
			    gl_Position = myPMVMatrix * myVertex;
			    myTexCoord = myUV.st;
		    }
        );

    mProgramCube = glCreateProgram();

    GLuint m_uiVertexShader, m_uiFragShader;
    m_uiVertexShader = CompileShader(GL_VERTEX_SHADER, vs);
    m_uiFragShader = CompileShader(GL_FRAGMENT_SHADER, fs);

    // Attach the fragment and vertex shaders to it
    glAttachShader(mProgramCube, m_uiFragShader);
    glAttachShader(mProgramCube, m_uiVertexShader);

    glBindAttribLocation(mProgramCube, VERTEX_ARRAY, "myVertex");
    glBindAttribLocation(mProgramCube, TEXCOORD_ARRAY, "myUV");

    glLinkProgram(mProgramCube);
    GLint bLinked;
    glGetProgramiv(mProgramCube, GL_LINK_STATUS, &bLinked);

    if (!bLinked)
    {
        int i32InfoLogLength, i32CharsWritten;
        glGetProgramiv(mProgramCube, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
        char* pszInfoLog = new char[i32InfoLogLength];
        glGetProgramInfoLog(mProgramCube, i32InfoLogLength, &i32CharsWritten, pszInfoLog);

        char* pszMsg = new char[i32InfoLogLength + 256];
        strcpy(pszMsg, "Failed to link program: ");
        strcat(pszMsg, pszInfoLog);


        std::cerr << "program link failed: " << pszMsg;
       delete[] pszMsg;
        delete[] pszInfoLog;
        glDeleteProgram(mProgramCube);

        return 0;
    }


    glUseProgram(mProgramCube);

    glUniform1i(glGetUniformLocation(mProgramCube, "sampler2d"), 0);

    // VBO handle
    GLuint m_ui32Vbo;

    GLuint* pTexData = new GLuint[TEX_SIZE*TEX_SIZE];
    for (int i = 0; i<TEX_SIZE; i++)
        for (int j = 0; j<TEX_SIZE; j++)
        {
            // Fills the data with a fancy pattern
            GLuint col = (255 << 24) + ((255 - j * 2) << 16) + ((255 - i) << 8) + (255 - i * 2);
            if (((i*j) / 8) % 2) col = (GLuint)(255 << 24) + (255 << 16) + (0 << 8) + (255);
            pTexData[j*TEX_SIZE + i] = col;
        }

    GLuint texture = 0;
    {
        sf::Image image;
        if (!image.loadFromFile("resources/texture.jpg"))
            return EXIT_FAILURE;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTexData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // Enable Z-buffer read and write
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    //glClearDepth(1.f);

    // Disable lighting
    //glDisable(GL_LIGHTING);

    // Configure the viewport (the same size as the window)
    glViewport(0, 0, window.getSize().x, window.getSize().y);

    // Setup a perspective projection
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    GLfloat ratio = static_cast<float>(window.getSize().x) / window.getSize().y;
    //glFrustum(-ratio, ratio, -1.f, 1.f, 1.f, 500.f);

    // Bind the texture
    glEnable(GL_TEXTURE_2D);


    // Define a 3D cube (6 faces made of 2 triangles composed by 3 vertices)
    static const GLfloat cube[] =
    {
        // positions    // texture coordinates
        -0.20, -0.20, -0.20, 0, 0,
        -0.20, 0.20, -0.20, 1, 0,
        -0.20, -0.20, 0.20, 0, 1,
        -0.20, -0.20, 0.20, 0, 1,
        -0.20, 0.20, -0.20, 1, 0,
        -0.20, 0.20, 0.20, 1, 1,

        0.20, -0.20, -0.20, 0, 0,
        0.20, 0.20, -0.20, 1, 0,
        0.20, -0.20, 0.20, 0, 1,
        0.20, -0.20, 0.20, 0, 1,
        0.20, 0.20, -0.20, 1, 0,
        0.20, 0.20, 0.20, 1, 1,

        -0.20, -0.20, -0.20, 0, 0,
        0.20, -0.20, -0.20, 1, 0,
        -0.20, -0.20, 0.20, 0, 1,
        -0.20, -0.20, 0.20, 0, 1,
        0.20, -0.20, -0.20, 1, 0,
        0.20, -0.20, 0.20, 1, 1,

        -0.20, 0.20, -0.20, 0, 0,
        0.20, 0.20, -0.20, 1, 0,
        -0.20, 0.20, 0.20, 0, 1,
        -0.20, 0.20, 0.20, 0, 1,
        0.20, 0.20, -0.20, 1, 0,
        0.20, 0.20, 0.20, 1, 1,

        -0.20, -0.20, -0.20, 0, 0,
        0.20, -0.20, -0.20, 1, 0,
        -0.20, 0.20, -0.20, 0, 1,
        -0.20, 0.20, -0.20, 0, 1,
        0.20, -0.20, -0.20, 1, 0,
        0.20, 0.20, -0.20, 1, 1,

        -0.20, -0.20, 0.20, 0, 0,
        0.20, -0.20, 0.20, 1, 0,
        -0.20, 0.20, 0.20, 0, 1,
        -0.20, 0.20, 0.20, 0, 1,
        0.20, -0.20, 0.20, 1, 0,
        0.20, 0.20, 0.20, 1, 1
    };

    GLfloat vertices[] = { -0.4f, -0.4f, 0.0f, // Pos
        0.0f, 0.0f,	  // UVs
        0.4f, -0.4f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.4f, 0.0f,
        0.5f, 1.0f };

    glGenBuffers(1, &m_ui32Vbo);

    unsigned int m_ui32VertexStride = 5 * sizeof(GLfloat); // 3 floats for the pos, 2 for the UVs

    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

    // Set the buffer's data
    glBufferData(GL_ARRAY_BUFFER, 36 * m_ui32VertexStride, cube, GL_STATIC_DRAW);

    // Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnable(GL_CULL_FACE);

    // Create a clock for measuring the time elapsed
    sf::Clock clock;

    // Start game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();

            // Escape key: exit
            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
                window.close();

            // Adjust the viewport when the window is resized
            if (event.type == sf::Event::Resized)
                glViewport(0, 0, event.size.width, event.size.height);
        }

        glClearColor(1.0, .3, .0, .9);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // We get the position of the mouse cursor, so that we can move the box accordingly
        float x =  sf::Mouse::getPosition(window).x * 200.f / window.getSize().x - 100.f;
        float y = -sf::Mouse::getPosition(window).y * 200.f / window.getSize().y + 100.f;


        //glUseProgram(mProgram);

        // Load the vertex data
        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        //glEnableVertexAttribArray(0);
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        // DRAW CUBE //
        glUseProgram(mProgramCube);

        // Matrix used for projection model view
        float afIdentity[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };

        // First gets the location of that variable in the shader using its name
        int i32Location = glGetUniformLocation(mProgramCube, "myPMVMatrix");

        // Then passes the matrix to that variable
        glUniformMatrix4fv(i32Location, 1, GL_FALSE, afIdentity);//


        glBindTexture(GL_TEXTURE_2D, texture);
        
        glEnableVertexAttribArray(VERTEX_ARRAY);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_ui32VertexStride, vertices); //vertices
        glEnableVertexAttribArray(TEXCOORD_ARRAY);
        glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, m_ui32VertexStride, vertices); //vertices
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Bind the VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

        // Pass the vertex data
        glEnableVertexAttribArray(VERTEX_ARRAY);
        glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, m_ui32VertexStride, 0);

        // Pass the texture coordinates data
        glEnableVertexAttribArray(TEXCOORD_ARRAY);
        glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, m_ui32VertexStride, (void*)(3 * sizeof(GLfloat)));

        // Draws a non-indexed triangle array
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Unbind the VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Finally, display the rendered frame on screen
        window.display();
    }

    // Don't forget to destroy our texture
    glDeleteTextures(1, &texture);

    return EXIT_SUCCESS;
}
