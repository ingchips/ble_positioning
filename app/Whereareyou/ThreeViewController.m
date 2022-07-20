//
//  ThreeViewController.m
//  Whereareyou
//
//  Created by yfan on 2022/4/28.
//

#import "ThreeViewController.h"



@interface ThreeViewController ()
//@property (nonatomic,strong)SCNNode *spherenode;
@property (nonatomic,strong)UILabel *tagLab;
@property (nonatomic,assign)BOOL statusSwith;
@property (nonatomic,strong)UISwitch *switch1;
@end

@implementation ThreeViewController

//- (SCNNode *)spherenode{
//    if (!_spherenode) {
//        SCNSphere *sphere = [SCNSphere sphereWithRadius:0.1];
//
////        SCNMaterial *material = [[SCNMaterial alloc] init];
////        material.diffuse.contents = [UIImage imageNamed:@"earth.jpg"];
////        sphere.materials = @[material];
//
//        _spherenode = [SCNNode nodeWithGeometry:sphere];
//
//    }
//    return _spherenode;
//}

//懒加载拍摄会话
- (ARSession *)arSession
{
    if(_arSession != nil)
    {
        return _arSession;
    }
    //1.创建会话
    _arSession = [[ARSession alloc] init];
    //2返回会话
    return _arSession;
}

//创建AR视图
- (ARSCNView *)arSCNView
{
    if (_arSCNView != nil) {
        return _arSCNView;
    }
    //1.创建AR视图
    _arSCNView = [[ARSCNView alloc] initWithFrame:self.view.bounds];
    //2.设置视图会话
    _arSCNView.session = self.arSession;
    //3.自动刷新灯光（3D游戏用到，此处可忽略）
    _arSCNView.automaticallyUpdatesLighting = YES;
    
    return _arSCNView;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"AR空间定位";
    
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    
    //1.将AR视图添加到当前视图
    [self.view addSubview:self.arSCNView];
    //2.开启AR会话（此时相机开始工作）
    self.arSessionConfiguration = [ARWorldTrackingConfiguration new];
    [self.arSession runWithConfiguration:self.arSessionConfiguration];
    
    self.tagLab = [[UILabel alloc]initWithFrame:CGRectMake(0, 80, [UIScreen mainScreen].bounds.size.width, 60)];
    self.tagLab.text = @"设备地址";
    self.tagLab.lineBreakMode = NSLineBreakByCharWrapping;
    self.tagLab.numberOfLines = 0;
    [self.tagLab setFont:[UIFont systemFontOfSize:11]];
    self.tagLab.textAlignment = NSTextAlignmentCenter;
    [self.arSCNView addSubview:self.tagLab];
    
    self.switch1 = [[UISwitch alloc] init];
    self.switch1.frame = CGRectMake(([UIScreen mainScreen].bounds.size.width-40)/2-20, 150, 100, 130);
    self.switch1.tintColor = [UIColor blackColor];
    self.switch1.on = NO;
//    _statusSwith = NO;
    [self.arSCNView addSubview:self.switch1];
    [self.switch1 addTarget:self action:@selector(switchAction:) forControlEvents:UIControlEventValueChanged];

    
    //测试
//    self.spherenode.position = SCNVector3Make(0, 0, -5);
//    [self.arSCNView.scene.rootNode addChildNode:self.spherenode];
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    /*
    //1.使用场景加载scn文件（scn格式文件是一个基于3D建模的文件，使用3DMax软件可以创建，这里系统有一个默认的3D飞机）--------在右侧我添加了许多3D模型，只需要替换文件名即可
    SCNScene *scene = [SCNScene sceneNamed:@"art.scnassets/ship.scn"];
    //2.获取飞机节点（一个场景会有多个节点，此处我们只写，飞机节点则默认是场景子节点的第一个）
    //所有的场景有且只有一个根节点，其他所有节点都是根节点的子节点
    SCNNode *shipNode = scene.rootNode.childNodes[0];
    
    //椅子比较大，可以可以调整Z轴的位置让它离摄像头远一点，，然后再往下一点（椅子太高我们坐不上去）就可以看得全局一点
//    shipNode.position = SCNVector3Make(0, -1, -1);//x/y/z/坐标相对于世界原点，也就是相机位置
        
    //3.将飞机节点添加到当前屏幕中
    [self.arSCNView.scene.rootNode addChildNode:shipNode];
    */

    
    
//    float x = 0;
//    float y = 0;
//    float z = -1;
//
//    SCNAction *action = [SCNAction moveTo:SCNVector3Make(0+x,-1+y,-1+z) duration:0];
//
//    [self.spherenode runAction:action];
//
//    [self.arSCNView.scene.rootNode addChildNode:self.spherenode];
}


- (void)switchAction:(UISwitch *)s1{
    if (s1.on == YES) {
        NSLog(@"开");
        _statusSwith = NO;
    }else{
        NSLog(@"关");
        _statusSwith = YES;
    }
}

- (void)PassValue:(NSDictionary *)localDic{
    NSArray *tagArr = [localDic objectForKey:@"id"];
    self.tagLab.text = [NSString stringWithFormat:@"%@ \nx:%@ \ny:%@ \nz:%@",tagArr[0],[localDic objectForKey:@"x"],[localDic objectForKey:@"y"],[localDic objectForKey:@"z"]];
    
    float x = [[localDic objectForKey:@"x"] floatValue];
    float y = [[localDic objectForKey:@"y"] floatValue];
    float z = [[localDic objectForKey:@"z"] floatValue];
    
    
//    float localX = x*0;
    
//    shipNode.position = SCNVector3Make(x,y, z);//x/y/z/坐标相对于世界原点，也就是相机位置
    //在手机前的坐标是(0, -1, -1)，定位偏移在此坐标基础变化;
    
//    SCNAction *action = [SCNAction moveTo:SCNVector3Make(localX, 1.5*(y-2), 1.5*(z-2)) duration:0.1];
    
    //这里修改成非懒加载，做运动轨迹
    //x设置成负数方便画轨迹
    if (self.switch1.on) {
        NSLog(@"开");
//        SCNAction *action = [SCNAction moveTo:SCNVector3Make(0.2+x,-0+y,-6-z) duration:0];
        SCNAction *action = [SCNAction moveTo:SCNVector3Make(0.2-x,-0+y,-6-0) duration:0];
        SCNSphere *sphere = [SCNSphere sphereWithRadius:0.06];
        SCNNode *_spherenode = [SCNNode nodeWithGeometry:sphere];
        [_spherenode runAction:action];
        
        [self.arSCNView.scene.rootNode addChildNode:_spherenode];
    }else{
        NSLog(@"关");
    }
    

}
/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
