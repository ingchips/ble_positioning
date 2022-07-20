//
//  TwoViewController.m
//  Whereareyou
//
//  Created by yfan on 2022/4/21.
//

#import "TwoViewController.h"
#import "OneViewController.h"

@interface TwoViewController (){
    float  localX;
    float  localY;
}
@property (nonatomic,strong)SCNView *scnView;
@property (nonatomic,strong)SCNNode *spherenode;
@property (weak, nonatomic) IBOutlet UILabel *tagLab;

@end

@implementation TwoViewController

- (SCNView *)scnView{
    if (!_scnView) {
        _scnView = [[SCNView alloc] init];
    }
    return _scnView;
}


- (SCNNode *)spherenode{
    if (!_spherenode) {
        _spherenode = [self.scnView.scene.rootNode childNodeWithName:@"sphere" recursively:YES];
    }
    return _spherenode;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"三维3D定位";
//    self.view.backgroundColor = UIColor.orangeColor;
    [self initSceneView];
    
    [self setCamera];
}

- (void)initSceneView{
//    SCNView *_scnView = [[SCNView alloc] init];

    SCNScene *scene = [SCNScene sceneNamed:@"man.dae"];
    
    self.scnView.scene = scene;
    self.scnView.frame = CGRectMake(0, 130, [UIScreen mainScreen].bounds.size.width, [UIScreen mainScreen].bounds.size.height - 130);
    self.scnView.backgroundColor = UIColor.whiteColor;
    self.scnView.allowsCameraControl = true;
    [self.view addSubview:self.scnView];
    
    
//    SCNNode *spherenode = [self.scnView.scene.rootNode childNodeWithName:@"_123" recursively:YES];
//
//    SCNAction *action = [SCNAction moveTo:SCNVector3Make(0, 1, 0) duration:2];
//    [spherenode runAction:action];
//    [self.scnView.scene.rootNode addChildNode:spherenode];
    
    
    
    SCNCamera *camera = [[SCNCamera alloc] init];
    
    SCNNode *cameraNode = [[SCNNode alloc] init];
    cameraNode.camera = camera;
    cameraNode.position = SCNVector3Make(0, 0, 30);
    [self.scnView.scene.rootNode addChildNode:cameraNode];

}

- (void)createSphereNode:(NSDictionary *)local{
    float x = [[local objectForKey:@"x"] floatValue];
    float y = [[local objectForKey:@"y"] floatValue];
    float z = [[local objectForKey:@"z"] floatValue];
    
    SCNAction *action = [SCNAction moveTo:SCNVector3Make(x, y, z) duration:0];
    [self.spherenode runAction:action];
    [self.scnView.scene.rootNode addChildNode:self.spherenode];
    
//    SCNNode *spherenode = [_scnView.scene.rootNode childNodeWithName:@"_123" recursively:YES];
    
    
    
    
}


- (void)PassValue:(NSDictionary *)localDic{
    
    NSArray *tagArr = [localDic objectForKey:@"id"];
    self.tagLab.text = [NSString stringWithFormat:@"%@ \nx:%@ \ny:%@ \nz:%@",tagArr[0],[localDic objectForKey:@"x"],[localDic objectForKey:@"y"],[localDic objectForKey:@"z"]];
    [self createSphereNode:localDic];
    
//    float x = [[localDic objectForKey:@"x"] floatValue];
//    float y = [[localDic objectForKey:@"y"] floatValue];
    
    
//    localX = width/2-10 + x*20;
//    localY = height/2-10 - y*20;
//    local.frame = CGRectMake(localX, localY, 20, 20);
}

- (void)setCamera{
    
    
}


@end
