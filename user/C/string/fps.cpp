using UnityEngine;
using System.Collections;


public class Main : MonoBehaviour
{
	public Texture2D AimPoint;
	// Use this for initialization
	void Start()
	{
		Screen.showCursor = false;//ȡ�����Ĭ��ͼ��
	}

	// Update is called once per frame
	void Update()
	{

	}
	void OnGUI()
	{
		//AimPoint.width >> 1��(AimPoint.height >>������һλ���൱�ڳ���2
		//(x >> 1) �� (x / 2) �Ľ����һ����
		Rect  rect = new Rect(Input.mousePosition.x - (AimPoint.width >> 1),
		                      Screen.height - Input.mousePosition.y - (AimPoint.height >> 1),
		                      AimPoint.width,
		                      AimPoint.height);
		GUI.DrawTexture(rect, AimPoint);
	}
}
