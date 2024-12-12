#version 400

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormals;
layout(location = 2) in vec2 vTextCoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 modeltoworld;

out vec3 fragWorldPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
out mat3 tbnMat;
out vec2 UV;
out vec3 temp;


//�׸��� ����� ���ؼ��� �ش� fragment�� ī�޶� �������� ��� ��ġ���� �˾ƾ� �ϱ� ������
//�Ʒ��� ���� ����Ͽ� varying���� fragment�� �ѱ�
out vec4 LightSpacePos;
uniform mat4 LightTransform;
uniform vec3 LightPerspective;
uniform mat4 WorldToView;
out vec3 VL;
out mat4 TC;

void main()
{
    gl_Position = model * vPosition;

    fragWorldPos = (modeltoworld * vPosition).xyz; // World Space ��ġ
    fragNormal = normalize((modeltoworld * vec4(vNormals, 0.0)).xyz); // World Space ����
    fragTexCoord = vTextCoords;
    UV=fragTexCoord;

    mat4 worldtomodel=inverse(modeltoworld);

    // Tangent Space ��ȯ ���        
    vec3 Tan= normalize(tangent);    
    vec3 Bin = normalize(bitangent);   
    vec3 Nor =  normalize(cross(Tan,Bin));

    //goto object space matrix from tangents
    tbnMat = transpose(mat3(Tan, Bin, Nor));//columm major

    LightSpacePos= LightTransform*(inverse(WorldToView) * model * vPosition);
}